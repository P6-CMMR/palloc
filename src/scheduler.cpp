#include "scheduler.hpp"

#include <memory>

#include "ortools/linear_solver/linear_solver.h"

using namespace palloc;
using namespace operations_research;

SchedulerResult Scheduler::scheduleBatch(Environment &env, const Requests &requests) {
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

    // sum_{p in P} x_{rp} in {1, 0}: All request have at most 1 parking spot
    for (size_t i = 0; i < requestCount; ++i) {
        MPConstraint *parkingVisitedConstraint =
            solver->MakeRowConstraint(0, PARKING_NODES_TO_VISIT);
        for (size_t j = 0; j < numberOfParkings; ++j) {
            parkingVisitedConstraint->SetCoefficient(var[i][j], 1.0);
        }
    }

    // sum_{r in R} x_{rp} <= c_p: number of requests in a parking node must be less or equal to its
    // capacity
    auto &availableParkingSpots = env.getAvailableParkingSpots();
    for (size_t j = 0; j < numberOfParkings; ++j) {
        MPConstraint *capacityConstraint =
            solver->MakeRowConstraint(0, static_cast<double>(availableParkingSpots[j]));
        for (size_t i = 0; i < requestCount; ++i) {
            capacityConstraint->SetCoefficient(var[i][j], 1.0);
        }
    }

    // sum_{r in R} sum_{p in P} (tau_P(r_d, p) + tau_D(p, r_d) < r_tau) -> (x_rp = 0):
    // if a duration to a parking spot is longer than the time request duration then the constrain
    // x_rp is 0
    for (size_t i = 0; i < requestCount; ++i) {
        const auto dropoffNode = requests[i].dropoffNode;
        const auto requestDuration = requests[i].duration;
        for (size_t j = 0; j < numberOfParkings; ++j) {
            const auto travelTime =
                (parkingToDropoff[j][dropoffNode] + dropoffToParking[dropoffNode][j]);
            if (travelTime > requestDuration) {
                MPConstraint *durationConstraint = solver->MakeRowConstraint(0, 0);
                durationConstraint->SetCoefficient(var[i][j], 1.0);
            }
        }
    }

    // (u + sum_{p in P} x_{rp}) = 1: All request have at most 1 parking spot or are unassigned
    std::vector<MPVariable *> unassignedVars(requestCount);
    for (size_t i = 0; i < requestCount; ++i) {
        unassignedVars[i] = solver->MakeBoolVar("u" + std::to_string(i));
        MPConstraint *unassignedConstraint = solver->MakeRowConstraint(1.0, 1.0);
        unassignedConstraint->SetCoefficient(unassignedVars[i], 1.0);
        for (size_t j = 0; j < numberOfParkings; ++j) {
            unassignedConstraint->SetCoefficient(var[i][j], 1.0);
        }
    }

    // min sum_{r in R} sum_{p in P} ((tau_P(r_d, p) + tau_D(p, r_d) * x_rp) + (u * penalty)
    // minimize the cost of time for getting to parking based on time getting to parking and a large
    // penalty for being unassigned
    MPObjective *objective = solver->MutableObjective();
    for (size_t i = 0; i < requestCount; ++i) {
        objective->SetCoefficient(unassignedVars[i], UNASSIGNED_PENALTY);
        const auto dropoffNode = requests[i].dropoffNode;
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

    double sumDuration = 0;
    double averageDuration = 0;
    double cost = objective->Value();

    if (result == MPSolver::OPTIMAL || result == MPSolver::FEASIBLE) {
        for (size_t i = 0; i < requestCount; ++i) {
            const auto &request = requests[i];
            size_t parkingNode = 0;
            bool assigned = false;
            for (size_t j = 0; j < numberOfParkings; ++j) {
                if (var[i][j]->solution_value() > 0.5) {
                    parkingNode = j;
                    assigned = true;
                    sumDuration +=
                        static_cast<double>(dropoffToParking[request.dropoffNode][parkingNode]) +
                        static_cast<double>(parkingToDropoff[parkingNode][request.dropoffNode]);
                    break;
                }
            }

            if (assigned) {
                --availableParkingSpots[parkingNode];
                simulations.emplace_back(request.dropoffNode, parkingNode, request.duration);

            } else {
                unassignedRequests.push_back(request);
            }
        }
    }

    averageDuration =
        simulations.empty() ? 0 : sumDuration / static_cast<double>(simulations.size());

    return {simulations, unassignedRequests, averageDuration, cost};
}