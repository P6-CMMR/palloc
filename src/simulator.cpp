#include <numeric>
#include <print>
#include <execution> 

#include "simulator.hpp"

using namespace palloc;

void Simulator::simulate(const Environment &env, uint64_t timesteps) {
    std::println("Dropoff nodes: {}", env.getDropoffToParking().size());
    std::println("Parking nodes: {}", env.getParkingToDropoff().size());

    const auto &pCap = env.getParkingCapacities();
    std::println("Total parking capacity: {}", std::reduce(std::execution::par, 
                                                           pCap.begin(),
                                                           pCap.end()));

    std::println("Simulating {} timesteps...", timesteps);
    for (uint64_t i = 0; i < timesteps; ++i) { /*sim loop*/ }
}