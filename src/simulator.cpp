#include "simulator.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <numeric>
#include <print>

#include "request_generator.hpp"
#include "scheduler.hpp"

using namespace palloc;

void Simulator::simulate(Environment &env, const SimulatorOptions &options) {
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

    Simulations simulations;
    Traces traces;
    const auto start = std::chrono::high_resolution_clock::now();
    for (uint64_t timestep = 0; timestep < options.timesteps; ++timestep) {
        if (!simulations.empty()) {
            updateSimulations(simulations, env);
        }

        insertNewRequests(generator, requests);

        const bool isLastStep = timestep == options.timesteps - 1;
        const bool processBatch =
            !requests.empty() &&
            ((timestep % options.batchDelay == 0 && timestep != 0) || isLastStep);

        if (processBatch) {
            const auto newSimulations = Scheduler::scheduleBatch(env, requests);
            requests.clear();
            if (!newSimulations.empty()) {
                simulations.insert(simulations.end(), newSimulations.begin(), newSimulations.end());
            }

            if (!isLastStep) {
                traces.emplace_back(
                    timestep, requests.size(), simulations.size(),
                    std::reduce(availableParkingSpots.begin(), availableParkingSpots.end()));
                ++timestep;
                insertNewRequests(generator, requests);
            }
        }

        traces.emplace_back(
            timestep, requests.size(), simulations.size(),
            std::reduce(availableParkingSpots.begin(), availableParkingSpots.end()));
    }

    const auto end = std::chrono::high_resolution_clock::now();
    const auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    for (const auto &trace : traces) {
        std::cout << trace << '\n';
    }

    std::println("Finished after {}ms", time);
}

void Simulator::updateSimulations(Simulations &simulations, Environment &env) {
    const auto &dropoffToParking = env.getDropoffToParking();
    const auto &parkingToDropoff = env.getParkingToDropoff();
    auto &availableParkingSpots = env.getAvailableParkingSpots();
    const auto simulate = [&dropoffToParking, &parkingToDropoff,
                           &availableParkingSpots](auto &simulation) {
        auto &durationLeft = simulation.durationLeft;
        --durationLeft;
        if (durationLeft == 0) {
            return true;
        }

        const auto &dropoffNode = simulation.dropoffNode;
        const auto &parkingNode = simulation.parkingNode;
        auto &currentNode = simulation.currentNode;

        if (currentNode == dropoffNode) {
            uint64_t timeToParking = dropoffToParking[currentNode][parkingNode] / 60;
            auto durationPassed = simulation.duration - durationLeft;
            if (durationPassed == timeToParking) {
                currentNode = parkingNode;
            }
        } else if (currentNode == parkingNode) {
            uint64_t timeToDrive = parkingToDropoff[parkingNode][dropoffNode] / 60;
            if (durationLeft == timeToDrive) {
                currentNode = dropoffNode;
                ++availableParkingSpots[parkingNode];
            }
        }

        return false;
    };

    const auto [first, last] = std::ranges::remove_if(simulations, simulate);
    simulations.erase(first, last);
}

void Simulator::insertNewRequests(RequestGenerator &generator, Requests &requests) {
    const auto newRequests = generator.generate();
    requests.insert(requests.end(), newRequests.begin(), newRequests.end());
}
