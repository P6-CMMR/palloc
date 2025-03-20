#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <cstdint>
#include <filesystem>
#include <list>

#include "environment.hpp"
#include "glaze/glaze.hpp"
#include "request_generator.hpp"
#include "trace.hpp"

namespace palloc {
struct Simulation {
    uint64_t dropoffNode;
    uint64_t parkingNode;
    uint64_t duration;
    uint64_t durationLeft;
    uint64_t tillArrival;
    bool inDropoff{true};
    bool visitedParking{false};

    explicit Simulation(uint64_t dropoffNode, uint64_t parkingNode, uint64_t duration, u_int64_t tillArrival)
        : dropoffNode(dropoffNode),
          parkingNode(parkingNode),
          duration(duration),
          durationLeft(duration),
          tillArrival(tillArrival) {}
};

using Simulations = std::list<Simulation>;

struct SimulatorSettings {
    uint64_t timesteps;
    uint64_t maxRequestDuration;
    uint64_t maxTimeTillArrival;
    uint64_t maxRequestsPerStep;
    uint64_t batchInterval;
    uint64_t seed;
};

class Simulator {
   public:
    struct OutputSettings {
        std::filesystem::path path;
        bool prettify;
        bool log;
    };

    static void simulate(Environment &env, const SimulatorSettings &simSettings,
                         const OutputSettings &outputSettings);

   private:
    static void updateSimulations(Simulations &simulations, Environment &env);
    static void insertNewRequests(RequestGenerator &generator, Requests &requests);
    static void insertSimulations(Simulations &simulations, Simulations &newSimulations);
    static void removeDeadRequests(Requests &unassignedRequests);
    static void decrementArrivalTime(Requests &earlyRequests);
    static void seperateTooEarlyRequests(Requests &requests, uint64_t maxDuration, Requests &earlyRequests);
    static void cutImpossibleRequests(Requests &requests,
                                      const Environment::UintVector &smallestRoundTrips);
};
}  // namespace palloc

template <>
struct glz::meta<palloc::SimulatorSettings> {
    using T = palloc::SimulatorSettings;
    static constexpr auto value =
        glz::object("timesteps", &T::timesteps, "max_request_duration", &T::maxRequestDuration,
                    "max_time_till_arrival", &T::maxTimeTillArrival, "max_request_per_step", &T::maxRequestsPerStep, 
                    "batch_interval", &T::batchInterval, "seed", &T::seed);
};

#endif