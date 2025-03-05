#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include "environment.hpp"
#include "request_generator.hpp"

#include <cstdint>

namespace palloc {
class Simulator {
   public:
    struct Simulation {
        size_t dropoffNode;
        size_t parkingNode;
        size_t duration;
    };

    using Simulations = std::vector<Simulation>;

    struct SimulatorOptions {
        uint64_t timesteps;
        uint64_t maxDuration;
        uint64_t maxRequestsPerStep;
        uint64_t batchDelay;
        uint64_t seed;
    };

    static void simulate(const Environment &env, const SimulatorOptions &options);
    
   private:
    static void scheduleBatch(const Environment &env, const RequestGenerator::Requests &requests);
};
}  // namespace palloc

#endif