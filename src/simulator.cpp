#include "simulator.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <numeric>
#include <print>
#include <thread>

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
    const auto start = std::chrono::high_resolution_clock::now();
    for (Uint run = 0; run < numberOfThreads; ++run) {
        threads.emplace_back(worker);
    }

    for (auto &thread : threads) {
        thread.join();
    }

    const auto end = std::chrono::high_resolution_clock::now();
    const auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::println("Finished after {}ms", time);

    const Result result = Result::aggregateResults(results);

    std::println("Total requests generated: {}", result.getRequestsGenerated());
    std::println("Total requests scheduled: {}", result.getRequestsScheduled());
    std::println("Total requests unassigned: {}",
                 result.getRequestsGenerated() - result.getRequestsScheduled());
    std::println("Total requests dropped: {}", result.getDroppedRequests());

<<<<<<< Updated upstream
    const double globalTotalDuration = result.getGlobalTotalDuration();
=======
    const double globalTotalDuration = result.getDuration();
>>>>>>> Stashed changes
    const int minutes = static_cast<int>(globalTotalDuration);
    const int seconds = static_cast<int>((globalTotalDuration - minutes) * 60);
    std::println("Average roundtrip time: {}m {}s", minutes, seconds);

    std::println("Average objective cost: {}", result.getCost());

    if (!outputSettings.outputPath.empty()) {
        result.saveToFile(outputSettings.outputPath, outputSettings.prettify);
    }
}

static Uint calculateMaxDuration(const Requests &requests) {
    Uint maxDuration = 0;
    for (const Request &request : requests) {
        if (!request.isEarly() && request.getRequestDuration() > maxDuration) {
            maxDuration = request.getRequestDuration();
        }
    }

    return maxDuration;
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

    TraceList traces;
    size_t droppedRequests = 0;
<<<<<<< Updated upstream
    double runCost = 0.0;
    Uint runDuration = 0;
=======

    DoubleVector runCostVec;
    runCostVec.reserve(timesteps);

    Uint runDurationSum = 0;
>>>>>>> Stashed changes
    size_t requestsScheduled = 0;
    size_t totalProcessedRequests = 0;
    for (Uint timestep = 1; timestep <= timesteps; ++timestep) {
        Uint currentTimeOfDay = ((simSettings.startTime + timestep - 1) % 1440);

        updateSimulations(simulations, env);
        removeDeadRequests(unassignedRequests);
        decrementArrivalTime(earlyRequests);
        insertNewRequests(generator, currentTimeOfDay, requests);
        cutImpossibleRequests(requests, env.getSmallestRoundTrips());

        double totalBatchCost = 0.0;
        Uint totalBatchDuration = 0;
<<<<<<< Updated upstream
=======
        size_t processedRequests = 0;
>>>>>>> Stashed changes
        Assignments assignments;

        bool isBatchingStep = timestep % simSettings.batchInterval == 0 || timestep == timesteps;
        if (isBatchingStep) {
            requests.insert(requests.end(), unassignedRequests.begin(), unassignedRequests.end());
            requests.insert(requests.end(), earlyRequests.begin(), earlyRequests.end());

            unassignedRequests.clear();
            earlyRequests.clear();

            Uint maxDuration = calculateMaxDuration(requests);
            seperateTooEarlyRequests(requests, maxDuration, earlyRequests);

            if (!requests.empty()) {
                const auto batchResult = Scheduler::scheduleBatch(env, requests, simSettings);
                requests.clear();

                totalBatchCost = batchResult.totalCost;
<<<<<<< Updated upstream
=======
                processedRequests = batchResult.processedRequests;
                totalProcessedRequests += processedRequests;
>>>>>>> Stashed changes
                totalBatchDuration = batchResult.totalDuration;

                unassignedRequests = batchResult.unassignedRequests;
                droppedRequests += unassignedRequests.size();

                earlyRequests = batchResult.earlyRequests;

                const auto &newSimulations = batchResult.simulations;
                assignments = createAssignments(newSimulations, env);

                simulations.insert(simulations.end(), newSimulations.begin(), newSimulations.end());
                requestsScheduled += newSimulations.size();
            }
        }

        const auto totalAvailableParkingSpots =
            std::reduce(availableParkingSpots.begin(), availableParkingSpots.end());

        if (outputSettings.outputTrace) {
            double batchAverageDuration = simulations.empty()
                                              ? 0.0
                                              : static_cast<double>(totalBatchDuration) /
                                                    static_cast<double>(simulations.size());
            double batchAverageCost =
                simulations.empty()
                    ? 0.0
<<<<<<< Updated upstream
                    : static_cast<double>(totalBatchCost) / static_cast<double>(simulations.size());
=======
                    : static_cast<double>(totalBatchCost) / static_cast<double>(processedRequests);
>>>>>>> Stashed changes
            traces.emplace_back(timestep, currentTimeOfDay, requests.size(), simulations.size(),
                                totalAvailableParkingSpots, batchAverageCost, batchAverageDuration,
                                droppedRequests, earlyRequests.size(), assignments);
        }

<<<<<<< Updated upstream
        runCost += totalBatchCost;
        runDuration += totalBatchDuration;
=======
        runCostVec.push_back(totalBatchCost);
        runDurationSum += totalBatchDuration;
>>>>>>> Stashed changes
    }

    assert(requests.empty());

<<<<<<< Updated upstream
    const Uint numberOfScheduledSteps =
        (timesteps + simSettings.batchInterval - 1) / simSettings.batchInterval;
    const double runAvgCost = runCost / static_cast<double>(requestsScheduled);

=======
>>>>>>> Stashed changes
    const Uint requestsGenerated = generator.getRequestsGenerated();

    const size_t requestsUnassigned = requestsGenerated - requestsScheduled;

    const std::lock_guard<std::mutex> guard(resultsMutex);

<<<<<<< Updated upstream
    results.emplace_back(traces, simSettings, droppedRequests, runDuration, runAvgCost,
                         requestsGenerated, requestsScheduled, requestsUnassigned);
=======
    double runCostSum = utils::KahanSum(runCostVec);
    results.emplace_back(traces, simSettings, droppedRequests, runDurationSum, runCostSum,
                         requestsGenerated, requestsScheduled, requestsUnassigned,
                         totalProcessedRequests);
>>>>>>> Stashed changes
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

void Simulator::seperateTooEarlyRequests(Requests &requests, Uint maxDuration,
                                         Requests &earlyRequests) {
    const auto end = requests.rend();
    for (auto it = requests.rbegin(); it != end; ++it) {
        if (it->getArrival() > 0) {
            earlyRequests.push_back(*it);
        }

        if (maxDuration >= it->getArrival()) {
            continue;
        }

        requests.erase(std::next(it).base());
    }
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
