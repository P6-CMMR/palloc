#include "scheduler.hpp"

#include <memory>

#include "ortools/sat/cp_model.h"

using namespace palloc;
using namespace operations_research;

SchedulerResult Scheduler::scheduleBatch(Environment &env, Requests &requests,
                                         const SimulatorSettings &simSettings) {
    assert(!requests.empty());

    sat::CpModelBuilder cpModel;

    const auto &parkingToDropoff = env.getParkingToDropoff();
    const auto &dropoffToParking = env.getDropoffToParking();
    const auto numberOfParkings = env.getNumberOfParkings();
    const auto requestCount = requests.size();
    auto &availableParkingSpots = env.getAvailableParkingSpots();

    const auto commitInterval = simSettings.commitInterval;

    // Binary variables from request to parkings
    std::vector<std::vector<sat::BoolVar>> var(requestCount);
    for (size_t i = 0; i < requestCount; ++i) {
        var[i].reserve(numberOfParkings);
        for (size_t j = 0; j < numberOfParkings; ++j) {
            var[i].push_back(cpModel.NewBoolVar());
        }
    }

    // Respect parking lot capacity
    for (size_t j = 0; j < numberOfParkings; ++j) {
        std::vector<sat::BoolVar> colVars;
        colVars.reserve(requestCount);
        for (size_t i = 0; i < requestCount; ++i) {
            colVars.push_back(var[i][j]);
        }

        cpModel.AddLessOrEqual(sat::LinearExpr::Sum(colVars), availableParkingSpots[j]);
    }

    // If travel time longer than request duration it cannot be assigned from r -> p
    const auto minParkingTime = simSettings.minParkingTime;
    for (size_t i = 0; i < requestCount; ++i) {
        const auto dropoffNode = requests[i].getDropoffNode();
        const auto requestDuration = requests[i].getRequestDuration();
        for (size_t j = 0; j < numberOfParkings; ++j) {
            const auto travelTime =
                (parkingToDropoff[j][dropoffNode] + dropoffToParking[dropoffNode][j]);
            if (travelTime + minParkingTime > requestDuration) {
                cpModel.AddEquality(var[i][j], 0);
            }
        }
    }

    // All request have at most 1 parking spot or are unassigned
    std::vector<sat::BoolVar> unassignedVars(requestCount);
    for (size_t i = 0; i < requestCount; ++i) {
        unassignedVars[i] = cpModel.NewBoolVar();

        std::vector<sat::BoolVar> combinedVars;
        combinedVars.reserve(numberOfParkings);
        combinedVars.push_back(unassignedVars[i]);
        for (size_t j = 0; j < numberOfParkings; ++j) {
            combinedVars.push_back(var[i][j]);
        }

        cpModel.AddEquality(sat::LinearExpr::Sum(combinedVars), 1);
    }

    // Minimize global cost of all requests
    sat::LinearExpr objective;
    const bool useWeightedParking = simSettings.useWeightedParking;
    for (size_t i = 0; i < requestCount; ++i) {
        const auto dropoffNode = requests[i].getDropoffNode();
        const auto dropFactor = 1 + requests[i].getTimesDropped();
        const auto penalty = UNASSIGNED_PENALTY * dropFactor;
        objective += penalty * sat::LinearExpr(unassignedVars[i]);
        for (size_t j = 0; j < numberOfParkings; ++j) {
            double cost = dropoffToParking[dropoffNode][j] + parkingToDropoff[j][dropoffNode];
            if (useWeightedParking) {
                const auto &parkingWeights = env.getParkingWeights();
                assert(parkingWeights[j] >= 0.0 && parkingWeights[j] <= 1.0);
                cost *= parkingWeights[j];
            }

            objective += std::lround(cost) * sat::LinearExpr(var[i][j]);
        }
    }

    cpModel.Minimize(objective);

    sat::Model model;
    sat::SatParameters parameters;
    parameters.set_max_time_in_seconds(MAX_SEARCH_TIME);
    parameters.set_num_search_workers(1);
    model.Add(sat::NewSatParameters(parameters));

    const sat::CpSolverResponse response = sat::SolveCpModel(cpModel.Build(), &model);

    Simulations simulations;
    Requests unassignedRequests;
    Requests earlyRequests;

    uint32_t sumDuration = 0;
    double averageDuration = 0.0;
    double cost = 0.0;

    if (response.status() == sat::CpSolverStatus::OPTIMAL ||
        response.status() == sat::CpSolverStatus::FEASIBLE) {
        cost = response.objective_value();
        for (size_t i = 0; i < requestCount; ++i) {
            auto &request = requests[i];
            size_t parkingNode = 0;
            bool assigned = false;
            const auto dropoffNode = request.getDropoffNode();
            const auto requestDuration = request.getRequestDuration();
            const auto tillArrival = request.getArrival();
            uint32_t routeDuration = 0;

            for (size_t j = 0; j < numberOfParkings; ++j) {
                if (sat::SolutionBooleanValue(response, var[i][j])) {
                    parkingNode = j;
                    assigned = true;
                    routeDuration = dropoffToParking[dropoffNode][parkingNode] +
                                    parkingToDropoff[parkingNode][dropoffNode];
                    break;
                }
            }

            sumDuration += routeDuration;
            if (tillArrival > commitInterval) {
                earlyRequests.push_back(request);
            } else if (assigned) {
                --availableParkingSpots[parkingNode];
                simulations.emplace_back(dropoffNode, parkingNode, requestDuration, tillArrival,
                                         routeDuration);
            } else {
                if (tillArrival > 0) {
                    earlyRequests.push_back(request);
                } else {
                    unassignedRequests.push_back(request);
                    request.incrementTimesDropped();
                }
            }
        }
    }

    averageDuration = simulations.empty() ? 0.0
                                          : static_cast<double>(sumDuration) /
                                                static_cast<double>(simulations.size());

    return {simulations, unassignedRequests, earlyRequests, averageDuration, cost};
}