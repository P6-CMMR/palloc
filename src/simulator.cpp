#include "simulator.hpp"

#include <numeric>
#include <print>
#include <chrono>
#include <limits>

#include "request_generator.hpp"
#include "ortools/linear_solver/linear_solver.h"

using namespace palloc;
using namespace operations_research;

void Simulator::simulate(const Environment &env, const SimulatorOptions &options) {
    const auto numberOfDropoffs = env.getNumberOfDropoffs();
    const auto numberOfParkings = env.getNumberOfParkings();
    std::println("Dropoff nodes: {}", numberOfDropoffs);
    std::println("Parking nodes: {}", numberOfParkings);

    const auto &pCap = env.getParkingCapacities();
    std::println("Total parking capacity: {}", std::reduce(pCap.begin(), pCap.end()));

    std::println("Simulating {} timesteps...", options.timesteps);
    RequestGenerator generator(numberOfDropoffs, options.maxDuration, options.maxRequestsPerStep, options.seed);
    RequestGenerator::Requests requests;
    requests.reserve(options.timesteps * options.maxRequestsPerStep / 2);

    const auto start = std::chrono::high_resolution_clock::now();
    for (uint64_t timestep = 0; timestep < options.timesteps; ++timestep) {
        const auto &newRequests = generator.generate();
        requests.insert(requests.end(), newRequests.begin(), newRequests.end());
        if ((timestep % options.batchDelay) == 0 && !requests.empty() && timestep != 0 || 
            timestep == options.timesteps - 1 && !requests.empty()) { 
            scheduleBatch(env, requests);
            requests.clear();
        }
    }

    const auto end = std::chrono::high_resolution_clock::now();
    const auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::println("Finished after {}ms", time);
}

void Simulator::scheduleBatch(const Environment &env, const RequestGenerator::Requests &requests) {
    auto solver = MPSolver::CreateSolver("SCIP");

    const double infinity = solver->infinity();
    const auto &parkingToDropoff = env.getParkingToDropoff();
    const auto &dropoffToParking = env.getDropoffToParking();
    const auto numberOfParkings = env.getNumberOfParkings();
    const auto requestCount = requests.size();
    
    std::vector<std::vector<MPVariable*>> x(requestCount);
    for (size_t i = 0; i < requestCount; ++i) {
        x[i].resize(numberOfParkings);
        for (size_t j = 0; j < numberOfParkings; ++j) {
            x[i][j] = solver->MakeBoolVar("x_" + std::to_string(i) + std::to_string(j));
        }
    }

    const double maxParking = 1.0;
    for (size_t i = 0; i < requestCount; ++i) {
        MPConstraint *constraint = solver->MakeRowConstraint(maxParking, maxParking);
        for (size_t j = 0; j < numberOfParkings; ++j) {
            constraint->SetCoefficient(x[i][j], maxParking);
        }
    }

    MPObjective *objective = solver->MutableObjective();
    for (size_t i = 0; i < requestCount; ++i) {
        const auto dropoffNode = requests[i].dropoffNode;
        for (auto j = 0; j < numberOfParkings; ++j) {
            const auto cost = dropoffToParking[dropoffNode][j] + parkingToDropoff[j][dropoffNode];
            objective->SetCoefficient(x[i][j], cost);
        }
    }

    objective->SetMinimization();
    const MPSolver::ResultStatus result = solver->Solve();
    assert(result == MPSolver::OPTIMAL || result == MPSolver::FEASIBLE);
}