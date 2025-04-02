#include "scheduler.hpp"

#include <memory>

#include "ortools/linear_solver/linear_solver.h"

using namespace palloc;
using namespace operations_research;

SchedulerResult Scheduler::scheduleBatch(Environment &env, Requests &requests) {
    assert(!requests.empty());

    std::unique_ptr<MPSolver> solver(MPSolver::CreateSolver("SCIP"));
    solver->set_time_limit(MAX_SEARCH_TIME);

    const auto &parkingToDropoff = env.getParkingToDropoff();
    const auto &dropoffToParking = env.getDropoffToParking();
    const auto numberOfParkings = env.getNumberOfParkings();
    const auto requestCount = requests.size();

    // x_{rp} in {0, 1}: Binary varables from request to parkings
    std::vector<std::vector<MPVariable *>> var(requestCount);
    for (size_t i = 0; i < requestCount; ++i) {
        var[i].reserve(numberOfParkings);
        for (size_t j = 0; j < numberOfParkings; ++j) {
            var[i].push_back(solver->MakeBoolVar(std::to_string(i) + std::to_string(j)));
        }
    }

    // All request have at most 1 parking spot
    for (size_t i = 0; i < requestCount; ++i) {
        MPConstraint *parkingVisitedConstraint =
            solver->MakeRowConstraint(0, PARKING_NODES_TO_VISIT);
        for (size_t j = 0; j < numberOfParkings; ++j) {
            parkingVisitedConstraint->SetCoefficient(var[i][j], 1.0);
        }
    }

    // Respect parking lot capacity
    auto &availableParkingSpots = env.getAvailableParkingSpots();
    for (size_t j = 0; j < numberOfParkings; ++j) {
        MPConstraint *capacityConstraint =
            solver->MakeRowConstraint(0, static_cast<double>(availableParkingSpots[j]));
        for (size_t i = 0; i < requestCount; ++i) {
            capacityConstraint->SetCoefficient(var[i][j], 1.0);
        }
    }

    // If travel time longer than request duration it cannot be assigned from r -> p
    for (size_t i = 0; i < requestCount; ++i) {
        const auto dropoffNode = requests[i].getDropoffNode();
        const auto requestDuration = requests[i].getRequestDuration();
        for (size_t j = 0; j < numberOfParkings; ++j) {
            const auto travelTime =
                (parkingToDropoff[j][dropoffNode] + dropoffToParking[dropoffNode][j]);
            if (travelTime > requestDuration) {
                MPConstraint *durationConstraint = solver->MakeRowConstraint(0, 0);
                durationConstraint->SetCoefficient(var[i][j], 1.0);
            }
        }
    }

    // All request have at most 1 parking spot or are unassigned
    std::vector<MPVariable *> unassignedVars(requestCount);
    for (size_t i = 0; i < requestCount; ++i) {
        unassignedVars[i] = solver->MakeBoolVar("u" + std::to_string(i));
        MPConstraint *unassignedConstraint = solver->MakeRowConstraint(1.0, 1.0);
        unassignedConstraint->SetCoefficient(unassignedVars[i], 1.0);
        for (size_t j = 0; j < numberOfParkings; ++j) {
            unassignedConstraint->SetCoefficient(var[i][j], 1.0);
        }
    }

    // Minimize global cost of all requests
    MPObjective *objective = solver->MutableObjective();
    for (size_t i = 0; i < requestCount; ++i) {
        const double dropFactor = (1.0 + static_cast<double>(requests[i].getTimesDropped()));
        const bool isEarly = requests[i].isEarly();
        objective->SetCoefficient(unassignedVars[i],
                                  UNASSIGNED_PENALTY * dropFactor * static_cast<double>(!isEarly));
        const auto dropoffNode = requests[i].getDropoffNode();
        for (size_t j = 0; j < numberOfParkings; ++j) {
            const double cost = static_cast<double>(dropoffToParking[dropoffNode][j] +
                                                    parkingToDropoff[j][dropoffNode]);
            objective->SetCoefficient(var[i][j], cost);
        }
    }

    objective->SetMinimization();
    const MPSolver::ResultStatus result = solver->Solve();
    assert(result == MPSolver::OPTIMAL || result == MPSolver::FEASIBLE);

    Simulations simulations;
    Requests unassignedRequests;
    Requests earlyRequests;

    uint64_t sumDuration = 0;
    double averageDuration = 0.0;
    double cost = objective->Value();
    if (result == MPSolver::OPTIMAL || result == MPSolver::FEASIBLE) {
        for (size_t i = 0; i < requestCount; ++i) {
            auto &request = requests[i];
            size_t parkingNode = 0;
            bool assigned = false;
            const auto dropoffNode = request.getDropoffNode();
            const auto requestDuration = request.getRequestDuration();
            const auto tillArrival = request.getArrival();
            uint64_t routeDuration = 0.0;
            for (size_t j = 0; j < numberOfParkings; ++j) {
                if (var[i][j]->solution_value() > 0.5) {
                    parkingNode = j;
                    assigned = true;
                    routeDuration = dropoffToParking[dropoffNode][parkingNode] +
                                    parkingToDropoff[parkingNode][dropoffNode];
                    break;
                }
            }

            sumDuration += routeDuration;

            if (tillArrival > 0) {
                earlyRequests.push_back(request);
            } else if (assigned) {
                --availableParkingSpots[parkingNode];
                simulations.emplace_back(dropoffNode, parkingNode, requestDuration, routeDuration);
            } else {
                unassignedRequests.push_back(request);
                request.incrementTimesDropped();
            }
        }
    }

    averageDuration = simulations.empty() ? 0.0
                                          : static_cast<double>(sumDuration) /
                                                static_cast<double>(simulations.size());

    return {simulations, unassignedRequests, earlyRequests, averageDuration, cost};
}