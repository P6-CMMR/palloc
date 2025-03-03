#include "simulator.hpp"

#include <execution>
#include <numeric>
#include <print>
#include <chrono>

#include "request_generator.hpp"

using namespace palloc;

void Simulator::simulate(const Environment &env, uint64_t timesteps, uint64_t maxDuration,
    uint64_t maxRequestsPerStep, uint64_t batchDelay, uint64_t seed) {
    size_t dropoffNodes = env.getDropoffToParking().size();
    size_t parkingNodes = env.getParkingToDropoff().size();
    std::println("Dropoff nodes: {}", dropoffNodes);
    std::println("Parking nodes: {}", parkingNodes);

    const auto &pCap = env.getParkingCapacities();
    std::println("Total parking capacity: {}",
                 std::reduce(std::execution::unseq, pCap.begin(), pCap.end()));

    std::println("Simulating {} timesteps...", timesteps);
    RequestGenerator generator(dropoffNodes, maxDuration, maxRequestsPerStep, seed);
    RequestGenerator::Requests requests;
    const auto start = std::chrono::high_resolution_clock::now();
    for (uint64_t timestep = 0; timestep < timesteps; ++timestep) {
        const auto &newRequests = generator.generate();
        requests.insert(requests.end(), newRequests.begin(), newRequests.end());
        if ((timestep % batchDelay) == 0) { 
            scheduleBatch(env, requests);
            requests.clear();
        }


    }

    const auto end = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end - start).count();

    std::println("Finished after {}µs", duration);
}

void Simulator::scheduleBatch(const Environment &env, const RequestGenerator::Requests &requests) {
}