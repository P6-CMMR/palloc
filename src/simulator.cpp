#include "simulator.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <numeric>
#include <print>

#include "result.hpp"
#include "scheduler.hpp"

using namespace palloc;

void Simulator::simulate(Environment &env, const SimulatorSettings &simSettings,
                         const OutputSettings &outputSettings) {
    assert(simSettings.timesteps > 0);
    assert(simSettings.maxRequestDuration > 0);

    const auto numberOfDropoffs = env.getNumberOfDropoffs();
    const auto numberOfParkings = env.getNumberOfParkings();
    std::println("Dropoff nodes: {}", numberOfDropoffs);
    std::println("Parking nodes: {}", numberOfParkings);

    const auto &availableParkingSpots = env.getAvailableParkingSpots();
    std::println("Total parking capacity: {}",
                 std::reduce(availableParkingSpots.begin(), availableParkingSpots.end()));

    const auto seed = simSettings.seed;
    std::println("Using seed: {}", seed);

    std::println("Simulating {} timesteps...", simSettings.timesteps);
    RequestGenerator generator(numberOfDropoffs, simSettings.maxRequestDuration,
                               simSettings.maxRequestsPerStep, seed);
    Requests requests;
    requests.reserve(simSettings.timesteps * simSettings.maxRequestsPerStep / 2);

    Requests unassignedRequests;
    size_t droppedRequests = 0;

    Simulations simulations;

    double globalCost = 0.0;
    double globalDuration = 0.0;

    Traces traces;
    const auto start = std::chrono::high_resolution_clock::now();
    for (uint64_t timestep = 1; timestep <= simSettings.timesteps; ++timestep) {
        updateSimulations(simulations, env);
        removeDeadRequests(unassignedRequests);
        insertNewRequests(generator, requests);
        cutImpossibleRequests(requests, env.getSmallestRoundTrips());

        double batchCost = 0.0;
        double batchAverageDuration = 0.0;
        if (!requests.empty() && (timestep % simSettings.batchInterval == 0)) {
            requests.insert(requests.end(), unassignedRequests.begin(), unassignedRequests.end());
            unassignedRequests.clear();

            auto result = Scheduler::scheduleBatch(env, requests);
            requests.clear();

            batchCost = result.cost;
            batchAverageDuration = result.averageDurations;

            unassignedRequests = result.unassignedRequests;
            droppedRequests += unassignedRequests.size();

            const auto &newSimulations = result.simulations;

            simulations.insert(simulations.end(), newSimulations.begin(), newSimulations.end());
        }

        const auto totalAvailableParkingSpots =
            std::reduce(availableParkingSpots.begin(), availableParkingSpots.end());
        traces.emplace_back(timestep, requests.size(), simulations.size(),
                            totalAvailableParkingSpots, batchCost, batchAverageDuration,
                            droppedRequests);

        globalCost += batchCost;
        globalDuration += batchAverageDuration;
    }

    const auto end = std::chrono::high_resolution_clock::now();
    const auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    if (outputSettings.log) {
        for (const auto &trace : traces) {
            std::cout << trace << '\n';
        }
    }

    std::println("Finished after {}ms", time);

    const double globalAvgDuration = globalDuration / static_cast<double>(simSettings.timesteps);
    const int minutes = static_cast<int>(globalAvgDuration);
    const int seconds = static_cast<int>((globalAvgDuration - minutes) * 60);
    std::println("Average roundtrip time: {}m {}s", minutes, seconds);

    const double globalAvgCost = globalCost / static_cast<double>(simSettings.timesteps);
    std::println("Average objective cost: {}", globalAvgCost);
    std::println("Total requests dropped: {}", droppedRequests);

    if (!outputSettings.path.empty()) {
        Result result(traces, simSettings, droppedRequests, globalAvgDuration, globalAvgCost);
        result.saveToFile(outputSettings.path, outputSettings.prettify);
    }
}

void Simulator::updateSimulations(Simulations &simulations, Environment &env) {
    const auto &dropoffToParking = env.getDropoffToParking();
    const auto &parkingToDropoff = env.getParkingToDropoff();
    auto &availableParkingSpots = env.getAvailableParkingSpots();
    const auto simulate = [&dropoffToParking, &parkingToDropoff,
                           &availableParkingSpots](auto &simulation) {
        auto &durationLeft = simulation.durationLeft;
        const auto &dropoffNode = simulation.dropoffNode;
        const auto &parkingNode = simulation.parkingNode;
        auto &inDropoff = simulation.inDropoff;
        auto &visitedParking = simulation.visitedParking;

        if (inDropoff && !visitedParking) {
            const auto timeToParking = dropoffToParking[dropoffNode][parkingNode];
            const auto durationPassed = simulation.duration - durationLeft;
            if (durationPassed == timeToParking) {
                inDropoff = false;
                visitedParking = true;
            }
        }

        const auto timeToDrive = parkingToDropoff[parkingNode][dropoffNode];
        if (!inDropoff && durationLeft == timeToDrive) {
            inDropoff = true;
            ++availableParkingSpots[parkingNode];
        }

        --durationLeft;
        if (durationLeft == 0 && !inDropoff && timeToDrive == 0) {
            inDropoff = true;
            ++availableParkingSpots[parkingNode];
        }

        assert(durationLeft || durationLeft == 0 && inDropoff);

        return durationLeft == 0;
    };

    const auto [first, last] = std::ranges::remove_if(simulations, simulate);
    simulations.erase(first, last);
}

void Simulator::insertNewRequests(RequestGenerator &generator, Requests &requests) {
    const auto newRequests = generator.generate();
    requests.insert(requests.end(), newRequests.begin(), newRequests.end());
}

void Simulator::removeDeadRequests(Requests &unassignedRequests) {
    std::erase_if(unassignedRequests, [](Request &request) {
        request.decrementDuration();
        return request.isDead();
    });
}

void Simulator::cutImpossibleRequests(Requests &requests,
                                      const Environment::UintVector &smallestRoundTrips) {
    std::erase_if(requests, [&smallestRoundTrips](Request &request) {
        assert(smallestRoundTrips.size() > request.getDropoffNode());
        return request.getDuration() < smallestRoundTrips[request.getDropoffNode()];
    });
}
