#include "simulator.hpp"

#include <numeric>
#include <print>
#include <chrono>
#include <limits>

#include "request_generator.hpp"
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
    RequestGenerator generator(numberOfDropoffs, numberOfParkings, options.maxDuration, options.maxRequestsPerStep, options.seed);
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
    const auto numParkings = env.getNumberOfParkings();
    const auto numDropoffs = env.getNumberOfDropoffs();

    std::vector<RoutingIndexManager::NodeIndex> dropoffIndices;
    dropoffIndices.reserve(requests.size());
    for (const auto &request : requests) {
        dropoffIndices.emplace_back(request.dropoffNode);
    }

    RoutingIndexManager manager(durationMatrix.size(), requests.size(), dropoffIndices, dropoffIndices);
    RoutingModel routing(manager);
    
    const int64_t maxTimePerVehicle = std::numeric_limits<int64_t>::max() / 2;
    const int transitCallbackIdx = routing.RegisterTransitCallback([&durationMatrix, &manager](const int64_t fromIdx, const int64_t toIdx) -> int64_t {
        const auto fromNode = manager.IndexToNode(fromIdx).value();
        const auto toNode = manager.IndexToNode(toIdx).value();
        const int64_t duration = durationMatrix[fromNode][toNode];
        return duration == -1 ? maxTimePerVehicle : duration;
    });

    routing.SetArcCostEvaluatorOfAllVehicles(transitCallbackIdx);
    
    const std::string timeStr = "Time";
    routing.AddDimension(
        transitCallbackIdx,
        0,
        maxTimePerVehicle,
        true,   
        timeStr
    );

    RoutingSearchParameters searchParameters = DefaultRoutingSearchParameters();
    searchParameters.set_first_solution_strategy(FirstSolutionStrategy::PATH_CHEAPEST_ARC);
    searchParameters.mutable_time_limit()->set_seconds(5);
    searchParameters.set_log_search(true);

    const Assignment *solution = routing.SolveWithParameters(searchParameters);
    if (solution) {
        std::println("Solution found:");
        int64_t totalDuration = 0;
        for (int request = 0; request < requests.size(); ++request) {
            if (!routing.IsVehicleUsed(*solution, request)) {
                std::println("Vehicle {} is not used in solution", request);
                continue;
            }
            
            int64_t startIdx = routing.Start(request);
            std::print("Route for request {}: ", request);
            
            int64_t routeDuration = 0;
            std::vector<int> routeNodes;
            
            while (!routing.IsEnd(startIdx)) {
                const auto nodeIdx = manager.IndexToNode(startIdx).value();
                routeNodes.push_back(nodeIdx);
                std::print("{} -> ", nodeIdx);
                
                int64_t previousIdx = startIdx;
                startIdx = solution->Value(routing.NextVar(startIdx));
                routeDuration += routing.GetArcCostForVehicle(previousIdx, startIdx, request);
            }
            
            auto nodeIdx = manager.IndexToNode(startIdx).value();
            routeNodes.push_back(nodeIdx);
            std::println("{}", nodeIdx);
            std::println("Duration: {}s", routeDuration);
            totalDuration += routeDuration;
        }
        
        std::println("Total duration: {}s", totalDuration);
    } else {
        std::println("No solution found!");
    }
}