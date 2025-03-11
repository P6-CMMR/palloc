#include "simulator.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <numeric>
#include <print>

#include "scheduler.hpp"
#include "result.hpp"

using namespace palloc;

void Simulator::simulate(Environment &env, const SimulatorOptions &options,
                         const std::filesystem::path &outputPath) {
    assert(options.timesteps > 0);
    assert(options.maxDuration > 0);

    const auto numberOfDropoffs = env.getNumberOfDropoffs();
    const auto numberOfParkings = env.getNumberOfParkings();
    std::println("Dropoff nodes: {}", numberOfDropoffs);
    std::println("Parking nodes: {}", numberOfParkings);

    const auto &availableParkingSpots = env.getAvailableParkingSpots();
    std::println("Total parking capacity: {}",
                 std::reduce(availableParkingSpots.begin(), availableParkingSpots.end()));

    std::println("Using seed: {}", options.seed);

    std::println("Simulating {} timesteps...", options.timesteps);
    RequestGenerator generator(numberOfDropoffs, options.maxDuration, options.maxRequestsPerStep,
                               options.seed);
    Requests requests;
    requests.reserve(options.timesteps * options.maxRequestsPerStep / 2);

    Requests unassignedRequests;
    size_t droppedRequests = 0;

    Simulations simulations;

    double globalCost = 0.0;
    double globalDuration = 0.0;

    Traces traces;
    const auto start = std::chrono::high_resolution_clock::now();
    for (uint64_t timestep = 1; timestep <= options.timesteps; ++timestep) {
        if (!simulations.empty()) {
            updateSimulations(simulations, env);
        }

        if (!unassignedRequests.empty()) {
            removeExpiredUnassignedRequests(unassignedRequests);
        }

        insertNewRequests(generator, requests);

        double batchCost = 0.0;
        double batchAverageDuration = 0.0;

        if (!requests.empty() && (timestep % options.batchDelay == 0)) {
            if (!unassignedRequests.empty()) {
                requests.insert(requests.end(), 
                                unassignedRequests.begin(),
                                unassignedRequests.end());

                unassignedRequests.clear();
            }

            auto result = Scheduler::scheduleBatch(env, requests);
            requests.clear();

            batchCost = result.cost;
            batchAverageDuration = result.averageDurations;

            unassignedRequests = result.unassignedRequests;
            droppedRequests += unassignedRequests.size();

            const auto &newSimulations = result.simulations;
            if (!newSimulations.empty()) {
                simulations.insert(simulations.end(), newSimulations.begin(), newSimulations.end());
            }
        }
                  
        traces.emplace_back(timestep, 
                            requests.size(), 
                            simulations.size(),
                            std::reduce(availableParkingSpots.begin(), availableParkingSpots.end()), 
                            batchCost, 
                            batchAverageDuration);

        globalCost += batchCost;
        globalDuration += batchAverageDuration;
    }

    const auto end = std::chrono::high_resolution_clock::now();
    const auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    for (const auto &trace : traces) {
        std::cout << trace << '\n';
    }

    std::println("Finished after {}ms", time);

    const double globalAvgDuration = globalDuration / static_cast<double>(options.timesteps);
    const int minutes = static_cast<int>(globalAvgDuration);
    const int seconds = static_cast<int>((globalAvgDuration - minutes) * 60);
    std::println("Average roundtrip time: {}m {}s", minutes, seconds);

    const double globalAvgCost = globalCost / static_cast<double>(options.timesteps);
    std::println("Average objective cost: {}", globalAvgCost);
    std::println("Total requests dropped: {}", droppedRequests);

    if (!outputPath.empty()) {
        Result result(traces, globalAvgDuration, globalAvgCost, droppedRequests);
        result.saveToFile(outputPath);
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

void Simulator::removeExpiredUnassignedRequests(Requests &unassignedRequests) {
    const auto isDead = [](Request &req) {
        --req.duration;
        return req.duration == 0;
    };

    const auto [first, last] = std::ranges::remove_if(unassignedRequests, isDead);
    unassignedRequests.erase(first, last);
}
