#include "simulator.hpp"

#include <numeric>
#include <print>
#include <chrono>
#include <limits>

#include "request_generator.hpp"
#include "ortools/base/logging.h"
#include "ortools/constraint_solver/constraint_solver.h"
#include "ortools/constraint_solver/routing.h"
#include "ortools/constraint_solver/routing_enums.pb.h"
#include "ortools/constraint_solver/routing_index_manager.h"
#include "ortools/constraint_solver/routing_parameters.h"

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
    RequestGenerator generator(numberOfParkings, options.maxDuration, options.maxRequestsPerStep, options.seed);
    RequestGenerator::Requests requests;
    requests.reserve(options.timesteps * options.maxRequestsPerStep / 2);
    const auto start = std::chrono::high_resolution_clock::now();
    for (uint64_t timestep = 0; timestep < options.timesteps; ++timestep) {
        const auto &newRequests = generator.generate();
        requests.insert(requests.end(), newRequests.begin(), newRequests.end());
        if ((timestep % options.batchDelay) == 0) { 
            scheduleBatch(env, requests);
            requests.clear();
        }


    }

    const auto end = std::chrono::high_resolution_clock::now();
    const auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::println("Finished after {}ms", time);
}

void Simulator::scheduleBatch(const Environment &env, const RequestGenerator::Requests &requests) {
    if (requests.empty()) return;

    const auto &durationMatrix = env.getDurationMatrix();
    const auto &parkingCapacities = env.getParkingCapacities();
    const auto numParkings = env.getNumberOfParkings();
    const auto numDropoffs = env.getNumberOfDropoffs();

    std::vector<RoutingIndexManager::NodeIndex> dropoffIndicies;
    dropoffIndicies.reserve(requests.size());
    for (const auto &request : requests) {
        dropoffIndicies.emplace_back(request.dropoffNode);
    }

    RoutingIndexManager manager(durationMatrix.size(), requests.size(), dropoffIndicies, dropoffIndicies);
    RoutingModel routing(manager);

    const int transitCallbackIdx = routing.RegisterTransitCallback(
        [&durationMatrix, &manager](const int64_t from_index,
                          const int64_t to_index) -> int64_t {
        const int from_node = manager.IndexToNode(from_index).value();
        const int to_node = manager.IndexToNode(to_index).value();
        return durationMatrix[from_node][to_node];
    });

    routing.SetArcCostEvaluatorOfAllVehicles(transitCallbackIdx);

    routing.AddDimension(transitCallbackIdx, 0, 3000,
        true,  // start cumul to zero
        "Distance");
    routing.GetMutableDimension("Distance")->SetGlobalSpanCostCoefficient(100);

    RoutingSearchParameters searchParams = DefaultRoutingSearchParameters();
    searchParams.set_first_solution_strategy(
        FirstSolutionStrategy::PATH_CHEAPEST_ARC);
    searchParams.mutable_time_limit()->set_seconds(60);
    const Assignment *solution = routing.SolveWithParameters(searchParams);

    if (solution != nullptr) {
        std::println("solution found");
        for (size_t i = 0; i < requests.size(); ++i) {

        }
    } else {
        std::println("no solution");
    }
}