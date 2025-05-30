#include "simulator.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <numeric>
#include <print>
#include <thread>

#include "aggregated_result.hpp"
#include "scheduler.hpp"
#include "utils.hpp"

using namespace palloc;

Uint Simulation::getDropoffNode() const noexcept { return _dropoffNode; }

Uint Simulation::getParkingNode() const noexcept { return _parkingNode; }

Uint Simulation::getRequestDuration() const noexcept { return _requestDuration; }

Uint Simulation::getDurationLeft() const noexcept { return _durationLeft; }

Uint Simulation::getRouteDuration() const noexcept { return _routeDuration; }

bool Simulation::isInDropoff() const noexcept { return _inDropoff; }

bool Simulation::hasVisitedParking() const noexcept { return _visitedParking; }

bool Simulation::isEarly() const noexcept { return _earlyTimeLeft > 0; }

bool Simulation::isDead() const noexcept { return _durationLeft == 0; }

void Simulation::setIsInDropoff(bool inDropoff) noexcept { _inDropoff = inDropoff; }

void Simulation::setHasVisitedParking(bool visitedParking) noexcept {
    _visitedParking = visitedParking;
}

void Simulation::decrementDuration() noexcept { --_durationLeft; }

void Simulation::decrementEarlyArrival() noexcept { --_earlyTimeLeft; }

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

    std::println("Using {} generator with seed: {}", simSettings.randomGenerator, simSettings.seed);

    Uint startHour = simSettings.startTime / 60;
    Uint startMin = simSettings.startTime % 60;
    std::println("Starting simulation from start time: {} ({:02d}:{:02d})", simSettings.startTime,
                 startHour, startMin);

    Uint timesteps = simSettings.timesteps;
    Uint numberOfRuns = outputSettings.numberOfRunsToAggregate;

    Uint numberOfThreads = generalSettings.numberOfThreads;
    if (numberOfRuns > 1) {
        std::println("Simulating {} timesteps aggregating {} runs using {} threads...", timesteps,
                     numberOfRuns, numberOfThreads);
    } else {
        std::println("Simulating {} timesteps...", timesteps);
    }

    Results results;
    results.reserve(numberOfRuns);
    std::mutex resultsMutex;
    std::atomic<Uint> atomicRunCounter{0};
    auto worker = [&]() {
        for (Uint run = atomicRunCounter.fetch_add(1); run < numberOfRuns;
             run = atomicRunCounter.fetch_add(1)) {
            Simulator::simulateRun(env, simSettings, outputSettings, results, resultsMutex, run);
        }
    };

    std::vector<std::thread> threads;
    threads.reserve(numberOfRuns);
    const auto startClock = std::chrono::high_resolution_clock::now();
    for (Uint run = 0; run < numberOfThreads; ++run) {
        threads.emplace_back(worker);
    }

    for (auto &thread : threads) {
        thread.join();
    }

    const auto endClock = std::chrono::high_resolution_clock::now();
    const auto timeElapsed = static_cast<Uint>(
        std::chrono::duration_cast<std::chrono::milliseconds>(endClock - startClock).count());

    std::println("Finished after {}ms", timeElapsed);

    AggregatedResult result(results);
    result.setTimeElapsed(timeElapsed);

    std::println("Total requests generated: {}", result.getTotalRequestsGenerated());
    std::println("Total requests scheduled: {}", result.getTotalRequestsScheduled());
    std::println("Total requests unassigned: {}",
                 result.getTotalRequestsGenerated() - result.getTotalRequestsScheduled());
    std::println("Total requests dropped: {}", result.getTotalDroppedRequests());

    const double avgDuration = result.getAvgDuration();
    const auto minutes = static_cast<Uint>(avgDuration);
    const auto seconds = static_cast<Uint>((avgDuration - minutes) * 60);
    std::println("Average roundtrip time: {}m {}s", minutes, seconds);

    std::println("Average objective cost: {}", result.getAvgCost());

    std::println("Average variable count: {}", result.getAvgVariableCount());

    if (!outputSettings.outputPath.empty()) {
        result.saveToFile(outputSettings.outputPath, outputSettings.prettify);
    }
}

static Assignments createAssignments(const Simulations &newSimulations, const Environment &env) {
    Assignments assignments;
    assignments.reserve(newSimulations.size());
    for (const auto &simulation : newSimulations) {
        assert(simulation.getRequestDuration() >= simulation.getRouteDuration());
        assignments.emplace_back(env.getDropoffCoordinates()[simulation.getDropoffNode()],
                                 env.getParkingCoordinates()[simulation.getParkingNode()],
                                 simulation.getRequestDuration(), simulation.getRouteDuration());
    }

    return assignments;
}

void Simulator::simulateRun(Environment env, const SimulatorSettings &simSettings,
                            const OutputSettings &outputSettings, Results &results,
                            std::mutex &resultsMutex, Uint runNumber) {
    const auto &availableParkingSpots = env.getAvailableParkingSpots();
    const auto numberOfDropoffs = env.getNumberOfDropoffs();

    RequestGenerator generator({.randomGenerator = simSettings.randomGenerator,
                                .dropoffNodes = numberOfDropoffs,
                                .maxTimeTillArrival = simSettings.maxTimeTillArrival,
                                .maxRequestDuration = simSettings.maxRequestDuration,
                                .seed = simSettings.seed + runNumber,
                                .requestRate = simSettings.requestRate});

    const Uint timesteps = simSettings.timesteps;

    Requests requests;
    requests.reserve(static_cast<size_t>(timesteps) *
                     static_cast<size_t>(std::ceil(simSettings.requestRate)));

    Requests unassignedRequests;
    Requests earlyRequests;

    Simulations simulations;

    DoubleVector runCostVec;
    runCostVec.reserve(timesteps);

    TraceList traces;
    size_t droppedRequests = 0;
    Uint runDurationSum = 0;
    size_t requestsScheduled = 0;
    size_t totalProcessedRequests = 0;
    size_t runTotalVariableCount = 0;
    for (Uint timestep = 1; timestep <= timesteps; ++timestep) {
        Uint currentTimeOfDay = ((simSettings.startTime + timestep - 1) % 1440);
        updateSimulations(simulations, env);
        removeDeadRequests(unassignedRequests);
        decrementArrivalTime(earlyRequests);
        insertNewRequests(generator, currentTimeOfDay, requests);
        cutImpossibleRequests(requests, env.getSmallestRoundTrips());

        double totalBatchCost = 0.0;
        Uint totalBatchDuration = 0;
        size_t processedRequests = 0;
        size_t batchScheduled = 0;
        size_t totalVariableCount = 0;
        Assignments assignments;

        bool isBatchingStep = timestep % simSettings.batchInterval == 0 || timestep == timesteps;
        if (isBatchingStep) {
            requests.insert(requests.end(), unassignedRequests.begin(), unassignedRequests.end());
            requests.insert(requests.end(), earlyRequests.begin(), earlyRequests.end());

            unassignedRequests.clear();
            earlyRequests.clear();

            if (!requests.empty()) {
                const auto batchResult = Scheduler::scheduleBatch(env, requests, simSettings);
                requests.clear();

                totalBatchCost = batchResult.totalCost;
                processedRequests = batchResult.processedRequests;
                totalProcessedRequests += processedRequests;
                totalBatchDuration = batchResult.totalDuration;

                unassignedRequests = batchResult.unassignedRequests;
                droppedRequests += unassignedRequests.size();

                earlyRequests = batchResult.earlyRequests;

                const auto &newSimulations = batchResult.simulations;
                assignments = createAssignments(newSimulations, env);

                simulations.insert(simulations.end(), newSimulations.begin(), newSimulations.end());
                batchScheduled += newSimulations.size();
                requestsScheduled += batchScheduled;

                totalVariableCount = batchResult.variableCount;
            }
        }

        const auto totalAvailableParkingSpots =
            std::reduce(availableParkingSpots.begin(), availableParkingSpots.end());

        if (outputSettings.outputTrace) {
            double batchAverageDuration =
                batchScheduled == 0
                    ? 0.0
                    : static_cast<double>(totalBatchDuration) / static_cast<double>(batchScheduled);
            double batchAverageCost = processedRequests == 0
                                          ? 0.0
                                          : totalBatchCost / static_cast<double>(processedRequests);

            Uint batchStepsCompleted = timestep / simSettings.batchInterval;
            if (timestep == timesteps && timestep % simSettings.batchInterval != 0) {
                ++batchStepsCompleted;
            }

            traces.emplace_back(assignments, requests.size(), simulations.size(),
                                totalAvailableParkingSpots, droppedRequests, earlyRequests.size(),
                                timestep, currentTimeOfDay, batchAverageCost, batchAverageDuration,
                                totalVariableCount);
        }

        runCostVec.push_back(totalBatchCost);
        runDurationSum += totalBatchDuration;
        runTotalVariableCount += totalVariableCount;
    }

    assert(requests.empty());

    const Uint requestsGenerated = generator.getRequestsGenerated();

    const size_t requestsUnassigned = requestsGenerated - requestsScheduled;

    const std::lock_guard<std::mutex> guard(resultsMutex);

    double runCostSum = utils::KahanSum(runCostVec);
    results.emplace_back(traces, simSettings, droppedRequests, runDurationSum, runCostSum,
                         runTotalVariableCount, requestsGenerated, requestsScheduled,
                         requestsUnassigned, totalProcessedRequests);
}

void Simulator::updateSimulations(Simulations &simulations, Environment &env) {
    const auto &dropoffToParking = env.getDropoffToParking();
    const auto &parkingToDropoff = env.getParkingToDropoff();
    auto &availableParkingSpots = env.getAvailableParkingSpots();
    const auto simulate = [&dropoffToParking, &parkingToDropoff,
                           &availableParkingSpots](auto &simulation) {
        if (simulation.isEarly()) {
            simulation.decrementEarlyArrival();
            return false;
        }

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
        if (simulation.isDead() && !simulation.isInDropoff() && timeToDrive == 0) {
            simulation.setIsInDropoff(true);
            ++availableParkingSpots[parkingNode];
        }

        assert(!simulation.isDead() || (simulation.isDead() && simulation.isInDropoff()));

        return simulation.isDead();
    };

    std::erase_if(simulations, simulate);
}

void Simulator::insertNewRequests(RequestGenerator &generator, Uint currentTimeOfDay,
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

void Simulator::decrementArrivalTime(Requests &earlyRequests) {
    for (auto &request : earlyRequests) {
        if (!request.isEarly()) {
            continue;
        }

        request.decrementTillArrival();
    }
}

void Simulator::cutImpossibleRequests(Requests &requests, const UintVector &smallestRoundTrips) {
    std::erase_if(requests, [&smallestRoundTrips](Request &request) {
        assert(smallestRoundTrips.size() > request.getDropoffNode());
        return request.getRequestDuration() < smallestRoundTrips[request.getDropoffNode()];
    });
}
