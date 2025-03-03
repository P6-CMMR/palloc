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
    size_t dropoffNodes = env.getDropoffToParking().size();
    size_t parkingNodes = env.getParkingToDropoff().size();
    std::println("Dropoff nodes: {}", dropoffNodes);
    std::println("Parking nodes: {}", parkingNodes);

    const auto &pCap = env.getParkingCapacities();
    std::println("Total parking capacity: {}", std::reduce(pCap.begin(), pCap.end()));

    std::println("Simulating {} timesteps...", options.timesteps);
    RequestGenerator generator(dropoffNodes, options.maxDuration, options.maxRequestsPerStep, options.seed);
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

    const auto &dropoffToParking = env.getDropoffToParking();
    const auto &parkingToDropoff = env.getParkingToDropoff();
    const auto numDropoffs = dropoffToParking.size();
    const auto numParkings = parkingToDropoff.size();
    const auto &parkingCapacities = env.getParkingCapacities();

    std::vector<RoutingIndexManager::NodeIndex> starts;
    std::vector<RoutingIndexManager::NodeIndex> ends;
    starts.reserve(requests.size());
    ends.reserve(requests.size());

    for (const auto &request : requests) {
        starts.emplace_back(request.dropoffNode);
    }
}