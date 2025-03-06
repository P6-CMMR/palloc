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
    for (uint64_t timestep = 1; timestep <= options.timesteps; ++timestep) {
        if (!simulations.empty()) {
            updateSimulations(simulations, env);
        }

        insertNewRequests(generator, requests);

        const bool isLastStep = timestep == options.timesteps;
        const bool processBatch =
            !requests.empty() &&
            ((timestep % options.batchDelay == 0) || isLastStep);

        if (processBatch) {
            const auto newSimulations = Scheduler::scheduleBatch(env, requests);
            requests.clear();
            if (!newSimulations.empty()) {
                simulations.insert(simulations.end(), newSimulations.begin(), newSimulations.end());
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
        const auto &dropoffNode = simulation.dropoffNode;
        const auto &parkingNode = simulation.parkingNode;
        auto &currentNode = simulation.currentNode;

        if (currentNode == dropoffNode) {
            auto timeToParking = dropoffToParking[currentNode][parkingNode] / SECONDS_TO_MINUTE;
            auto durationPassed = simulation.duration - durationLeft;
            if (durationPassed == timeToParking) {
                currentNode = parkingNode;
            }
        } 
        
        if (currentNode == parkingNode) {
            auto timeToDrive = parkingToDropoff[parkingNode][dropoffNode] / SECONDS_TO_MINUTE;
            if (durationLeft == timeToDrive) {
                currentNode = dropoffNode;
                ++availableParkingSpots[parkingNode];
            }
        }

        --durationLeft;
        return durationLeft == 0;
    };

    const auto [first, last] = std::ranges::remove_if(simulations, simulate);
    simulations.erase(first, last);
}

void Simulator::insertNewRequests(RequestGenerator &generator, Requests &requests) {
    const auto newRequests = generator.generate();
    requests.insert(requests.end(), newRequests.begin(), newRequests.end());
}
