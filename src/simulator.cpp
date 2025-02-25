#include "simulator.hpp"

using namespace palloc;

void Simulator::simulate(const Environment &env, uint64_t timesteps) {
    std::cout << "Simulating " << timesteps << " timesteps...\n";
    for (uint64_t i = 0; i < timesteps; ++i) {
        // generate random requests
        // if i mod b then send batch to or-tools
        // else add requests to batch
    }

    // print results to stdout
}