#include "scheduler.hpp"

#include <memory>

#include "ortools/linear_solver/linear_solver.h"

using namespace palloc;
using namespace operations_research;

Simulations Scheduler::scheduleBatch(Environment &env, const Requests &requests) {
    assert(!requests.empty());

    std::unique_ptr<MPSolver> solver(MPSolver::CreateSolver("SCIP"));
    solver->set_time_limit(MAX_SEARCH_TIME);

    const auto &parkingToDropoff = env.getParkingToDropoff();
    const auto &dropoffToParking = env.getDropoffToParking();
    const auto numberOfParkings = env.getNumberOfParkings();
    const auto requestCount = requests.size();

    std::vector<std::vector<MPVariable *>> var(requestCount);
    for (size_t i = 0; i < requestCount; ++i) {
        var[i].reserve(numberOfParkings);
        for (size_t j = 0; j < numberOfParkings; ++j) {
            var[i].push_back(solver->MakeBoolVar(std::to_string(i) + std::to_string(j)));
        }
    }

    for (size_t i = 0; i < requestCount; ++i) {
        MPConstraint *constraint = solver->MakeRowConstraint(PARKING_NODES_TO_VISIT, PARKING_NODES_TO_VISIT);
        for (size_t j = 0; j < numberOfParkings; ++j) {
            constraint->SetCoefficient(var[i][j], PARKING_NODES_TO_VISIT);
        }
    }

    auto &availableParkingSpots = env.getAvailableParkingSpots();
    for (size_t j = 0; j < numberOfParkings; ++j) {
        MPConstraint *capacityConstraint = 
            solver->MakeRowConstraint(0, static_cast<double>(availableParkingSpots[j]));
        for (size_t i = 0; i < requestCount; ++i) {
            capacityConstraint->SetCoefficient(var[i][j], 1.0);
        }
    }

    for (size_t i = 0; i < requestCount; ++i) {
        const auto dropoffNode = requests[i].dropoffNode;
        const auto requestDuration = requests[i].duration;
        for (size_t j = 0; j < numberOfParkings; ++j) {
            const double travelTime = static_cast<double>(parkingToDropoff[j][dropoffNode] + 
                                                        dropoffToParking[dropoffNode][j]);
            const double requestDurationInSeconds = requestDuration * 60.0;
            if (travelTime > requestDurationInSeconds) {
                MPConstraint *durationConstraint = solver->MakeRowConstraint(0, 0);
                durationConstraint->SetCoefficient(var[i][j], 1.0);
            }
        }
    }

    MPObjective *objective = solver->MutableObjective();
    for (size_t i = 0; i < requestCount; ++i) {
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
    if (result == MPSolver::OPTIMAL || result == MPSolver::FEASIBLE) {
        for (size_t i = 0; i < requestCount; ++i) {
            const auto &request = requests[i];
            size_t parkingNode = 0;
            for (size_t j = 0; j < numberOfParkings; ++j) {
                if (var[i][j]->solution_value() > 0.5) {
                    parkingNode = j;
                    break;
                }
            }

            --availableParkingSpots[parkingNode];
            simulations.emplace_back(request.dropoffNode, parkingNode, request.duration);
        }
    }

    return simulations;
}