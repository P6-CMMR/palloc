#include "simulator.hpp"

#include <execution>
#include <numeric>
#include <print>

<<<<<<< Updated upstream
using namespace palloc;

void Simulator::simulate(const Environment &env, uint64_t timesteps) {
    std::println("Dropoff nodes: {}", env.getDropoffToParking().size());
    std::println("Parking nodes: {}", env.getParkingToDropoff().size());
=======
#include "request_generator.hpp"

using namespace palloc;

void Simulator::simulate(const Environment &env, size_t timesteps, size_t maxDuration,
                         size_t maxRequestsPerStep, size_t seed) {
    size_t dropoffNodes = env.getDropoffToParking().size();
    size_t parkingNodes = env.getParkingToDropoff().size();
    std::println("Dropoff nodes: {}", dropoffNodes);
    std::println("Parking nodes: {}", parkingNodes);
>>>>>>> Stashed changes

    const auto &pCap = env.getParkingCapacities();
    std::println("Total parking capacity: {}",
                 std::reduce(std::execution::unseq, pCap.begin(), pCap.end()));

    std::println("Simulating {} timesteps...", timesteps);
<<<<<<< Updated upstream
    for (uint64_t i = 0; i < timesteps; ++i) { /*sim loop*/
=======
    RequestGenerator generator(dropoffNodes, maxDuration, maxRequestsPerStep, seed);
    for (uint64_t i = 0; i < timesteps; ++i) {
        for (const auto &[from, duration] : generator.generate()) {
            std::println("Request from {} with duration {}", from, duration);
        }
>>>>>>> Stashed changes
    }
}