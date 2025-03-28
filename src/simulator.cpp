#include "simulator.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <numeric>
#include <print>
#include <thread>

#include "scheduler.hpp"

using namespace palloc;

uint64_t Simulation::getDropoffNode() const noexcept { return _dropoffNode; }

uint64_t Simulation::getParkingNode() const noexcept { return _parkingNode; }

uint64_t Simulation::getRequestDuration() const noexcept { return _requestDuration; }

uint64_t Simulation::getDurationLeft() const noexcept { return _durationLeft; }

uint64_t Simulation::getRouteDuration() const noexcept { return _routeDuration; }

bool Simulation::isInDropoff() const noexcept { return _inDropoff; }

bool Simulation::hasVisitedParking() const noexcept { return _visitedParking; }

void Simulation::setIsInDropoff(bool inDropoff) noexcept { this->_inDropoff = inDropoff; }

void Simulation::setHasVisitedParking(bool visitedParking) noexcept {
    this->_visitedParking = visitedParking;
}

void Simulation::decrementDuration() noexcept { --_durationLeft; }

void Simulator::simulate(Environment &env, const SimulatorSettings &simSettings,
                         const OutputSettings &outputSettings,
                         const GeneralSettings &generalSettings) {
    assert(simSettings.timesteps > 0);
    assert(simSettings.requestRate > 0);
    assert(simSettings.startTime <= 1439);
    assert(outputSettings.numberOfRunsToAggregate > 0);

    const auto numberOfDropoffs = env.getNumberOfDropoffs();
    const auto numberOfParkings = env.getNumberOfParkings();
    std::println("Dropoff nodes: {}", numberOfDropoffs);
    std::println("Parking nodes: {}", numberOfParkings);

    const auto &availableParkingSpots = env.getAvailableParkingSpots();
    std::println("Total parking capacity: {}",
                 std::reduce(availableParkingSpots.begin(), availableParkingSpots.end()));

    const auto seed = simSettings.seed;
    std::println("Using seed: {}", seed);

    uint64_t startHour = simSettings.startTime / 60;
    uint64_t startMin = simSettings.startTime % 60;
    std::println("Starting simulation from start time: {} ({:02d}:{:02d})", simSettings.startTime,
                 startHour, startMin);

    uint64_t timesteps = simSettings.timesteps;
    uint64_t numberOfRuns = outputSettings.numberOfRunsToAggregate;

    uint64_t numberOfThreads = generalSettings.numberOfThreads;
    if (numberOfRuns > 1) {
        std::println("Simulating {} timesteps aggregating {} runs using {} threads...", timesteps,
                     numberOfRuns, numberOfThreads);
    } else {
        std::println("Simulating {} timesteps...", timesteps);
    }

    Results results;
    results.reserve(numberOfRuns);
    std::mutex resultsMutex;
    std::atomic<uint64_t> atomicRunCounter{0};
    auto worker = [&]() {
        for (uint64_t run = atomicRunCounter.fetch_add(1); run < numberOfRuns;
             run = atomicRunCounter.fetch_add(1)) {
            Simulator::simulateRun(env, simSettings, results, resultsMutex, run);
        }
    };

    std::vector<std::thread> threads;
    threads.reserve(numberOfRuns);
    const auto start = std::chrono::high_resolution_clock::now();
    for (uint64_t run = 0; run < numberOfThreads; ++run) {
        threads.emplace_back(worker);
    }

    for (auto &thread : threads) {
        thread.join();
    }

    const auto end = std::chrono::high_resolution_clock::now();
    const auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::println("Finished after {}ms", time);

    Result result = Result::aggregateResults(results);

    const double globalAvgDuration = result.getGlobalAvgDuration();
    const int minutes = static_cast<int>(globalAvgDuration);
    const int seconds = static_cast<int>((globalAvgDuration - minutes) * 60);
    std::println("Average roundtrip time: {}m {}s", minutes, seconds);

    const double globalAvgCost = result.getGlobalAvgCost();
    std::println("Average objective cost: {}", globalAvgCost);
    std::println("Total requests dropped: {}", result.getDroppedRequests());

    if (!outputSettings.path.empty()) {
        result.saveToFile(outputSettings.path, outputSettings.prettify);
    }
}

void Simulator::simulateRun(Environment env, const SimulatorSettings &simSettings, Results &results,
                            std::mutex &resultsMutex, uint64_t runNumber) {
    auto &availableParkingSpots = env.getAvailableParkingSpots();
    const auto numberOfDropoffs = env.getNumberOfDropoffs();

    RequestGenerator generator(numberOfDropoffs, simSettings.maxRequestDuration,
                               simSettings.seed + runNumber, simSettings.requestRate);

    uint64_t timesteps = simSettings.timesteps;

    Requests requests;
    requests.reserve(timesteps * static_cast<uint64_t>(std::ceil(simSettings.requestRate)));

    Requests unassignedRequests;
    Simulations simulations;

    TraceList traces;

    size_t droppedRequests = 0;
    double runCost = 0.0;
    double runDuration = 0.0;

    for (uint64_t timestep = 1; timestep <= timesteps; ++timestep) {
        uint64_t currentTimeOfDay = ((simSettings.startTime + timestep - 1) % 1440);

        updateSimulations(simulations, env);
        removeDeadRequests(unassignedRequests);
        insertNewRequests(generator, currentTimeOfDay, requests);
        cutImpossibleRequests(requests, env.getSmallestRoundTrips());

        double batchCost = 0.0;
        double batchAverageDuration = 0.0;
        Assignments assignments;
        if (!requests.empty() && (timestep % simSettings.batchInterval == 0)) {
            requests.insert(requests.end(), unassignedRequests.begin(), unassignedRequests.end());
            unassignedRequests.clear();

            const auto batchResult = Scheduler::scheduleBatch(env, requests);
            requests.clear();

            batchCost = batchResult.cost;
            batchAverageDuration = batchResult.averageDurations;

            unassignedRequests = batchResult.unassignedRequests;
            droppedRequests += unassignedRequests.size();

            const auto &newSimulations = batchResult.simulations;
            assignments.reserve(newSimulations.size());
            for (const auto &simulation : newSimulations) {
                assert(simulation.getRequestDuration() >= simulation.getRouteDuration());
                assignments.emplace_back(env.getDropoffCoordinates()[simulation.getDropoffNode()],
                                         env.getParkingCoordinates()[simulation.getParkingNode()],
                                         simulation.getRequestDuration(),
                                         simulation.getRouteDuration());
            }

            simulations.insert(simulations.end(), newSimulations.begin(), newSimulations.end());
        }

        const auto totalAvailableParkingSpots =
            std::reduce(availableParkingSpots.begin(), availableParkingSpots.end());
        traces.emplace_back(timestep, currentTimeOfDay, requests.size(), simulations.size(),
                            totalAvailableParkingSpots, batchCost, batchAverageDuration,
                            droppedRequests, assignments);

        runCost += batchCost;
        runDuration += batchAverageDuration;
    }

    const double runAvgDuration = runDuration / static_cast<double>(timesteps);
    const double runAvgCost = runCost / static_cast<double>(timesteps);

    std::lock_guard<std::mutex> guard(resultsMutex);
    results.emplace_back(traces, simSettings, droppedRequests, runAvgDuration, runAvgCost);
}

void Simulator::updateSimulations(Simulations &simulations, Environment &env) {
    const auto &dropoffToParking = env.getDropoffToParking();
    const auto &parkingToDropoff = env.getParkingToDropoff();
    auto &availableParkingSpots = env.getAvailableParkingSpots();
    const auto simulate = [&dropoffToParking, &parkingToDropoff,
                           &availableParkingSpots](auto &simulation) {
        const auto dropoffNode = simulation.getDropoffNode();
        const auto parkingNode = simulation.getParkingNode();

        if (simulation.isInDropoff() && !simulation.hasVisitedParking()) {
            const auto timeToParking = dropoffToParking[dropoffNode][parkingNode];
            const auto durationPassed =
                simulation.getRequestDuration() - simulation.getDurationLeft();
            if (durationPassed == timeToParking) {
                simulation.setIsInDropoff(false);
                simulation.setHasVisitedParking(true);
            }
        }

        const auto timeToDrive = parkingToDropoff[parkingNode][dropoffNode];
        if (!simulation.isInDropoff() && simulation.getDurationLeft() == timeToDrive) {
            simulation.setIsInDropoff(true);
            ++availableParkingSpots[parkingNode];
        }

        simulation.decrementDuration();
        if (simulation.getDurationLeft() == 0 && !simulation.isInDropoff() && timeToDrive == 0) {
            simulation.setIsInDropoff(true);
            ++availableParkingSpots[parkingNode];
        }

        assert(simulation.getDurationLeft() ||
               (simulation.getDurationLeft() == 0 && simulation.isInDropoff()));

        return simulation.getDurationLeft() == 0;
    };

    std::erase_if(simulations, simulate);
}

void Simulator::insertNewRequests(RequestGenerator &generator, uint64_t currentTimeOfDay,
                                  Requests &requests) {
    const auto newRequests = generator.generate(currentTimeOfDay);
    requests.insert(requests.end(), newRequests.begin(), newRequests.end());
}

void Simulator::removeDeadRequests(Requests &unassignedRequests) {
    std::erase_if(unassignedRequests, [](Request &request) {
        request.decrementDuration();
        return request.isDead();
    });
}

void Simulator::cutImpossibleRequests(Requests &requests, const UintVector &smallestRoundTrips) {
    std::erase_if(requests, [&smallestRoundTrips](Request &request) {
        assert(smallestRoundTrips.size() > request.getDropoffNode());
        return request.getRequestDuration() < smallestRoundTrips[request.getDropoffNode()];
    });
}
