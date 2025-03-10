#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <cstdint>
#include <list>

#include "environment.hpp"
#include "request_generator.hpp"
#include "trace.hpp"

namespace palloc {
struct Simulation {
    size_t dropoffNode;
    size_t parkingNode;
    uint64_t duration;
    uint64_t durationLeft;
    bool inDropoff{true};
    bool visitedParking{false};

    explicit Simulation(size_t dropoffNode, size_t parkingNode, uint64_t duration)
        : dropoffNode(dropoffNode),
          parkingNode(parkingNode),
          duration(duration),
          durationLeft(duration)
          {}
};

using Simulations = std::list<Simulation>;

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
    static void removeExpiredUnassignedRequests(Requests &unassignedRequests);
};
}  // namespace palloc

#endif