#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <cstdint>

#include "environment.hpp"
#include "request_generator.hpp"
#include "trace.hpp"

namespace palloc {
struct Simulation {
    size_t dropoffNode;
    size_t parkingNode;
    size_t currentNode;
    uint64_t duration;
    uint64_t durationLeft;

    explicit Simulation(size_t dropoffNode, size_t parkingNode, uint64_t duration)
        : dropoffNode(dropoffNode),
          parkingNode(parkingNode),
          currentNode(dropoffNode),
          duration(duration),
          durationLeft(duration) {}
};

using Simulations = std::vector<Simulation>;

class Simulator {
   public:
    struct SimulatorOptions {
        uint64_t timesteps;
        uint64_t maxDuration;
        uint64_t maxRequestsPerStep;
        uint64_t batchDelay;
        uint64_t seed;
    };

    static void simulate(Environment &env, const SimulatorOptions &options);

   private:
    static void updateSimulations(Simulations &simulations, Environment &env);
    static void insertNewRequests(RequestGenerator &generator, Requests &requests);
};
}  // namespace palloc

#endif