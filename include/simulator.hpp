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
class Simulation {
   public:
    explicit Simulation(uint64_t dropoffNode, uint64_t parkingNode, uint64_t requestDuration,
                        uint64_t routeDuration)
        : dropoffNode(dropoffNode),
          parkingNode(parkingNode),
          requestDuration(requestDuration),
          durationLeft(requestDuration),
          routeDuration(routeDuration) {}

    uint64_t getDropoffNode() const noexcept;
    uint64_t getParkingNode() const noexcept;
    uint64_t getRequestDuration() const noexcept;
    uint64_t getDurationLeft() const noexcept;
    uint64_t getRouteDuration() const noexcept;

    bool isInDropoff() const noexcept;
    bool hasVisitedParking() const noexcept;

    void setIsInDropoff(bool inDropoff) noexcept;
    void setHasVisitedParking(bool visitedParking) noexcept;

    void decrementDuration() noexcept;

   private:
    uint64_t dropoffNode;
    uint64_t parkingNode;
    uint64_t requestDuration;
    uint64_t durationLeft;
    uint64_t routeDuration;

    bool inDropoff{true};
    bool visitedParking{false};
};

using Simulations = std::list<Simulation>;

struct SimulatorSettings {
    uint64_t timesteps;
    uint64_t maxRequestDuration;
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
    static void removeDeadRequests(Requests &unassignedRequests);
    static void cutImpossibleRequests(Requests &requests,
                                      const Environment::UintVector &smallestRoundTrips);
};
}  // namespace palloc

template <>
struct glz::meta<palloc::SimulatorSettings> {
    using T = palloc::SimulatorSettings;
    static constexpr auto value =
        glz::object("timesteps", &T::timesteps, "max_request_duration", &T::maxRequestDuration,
                    "max_request_per_step", &T::maxRequestsPerStep, "batch_interval",
                    &T::batchInterval, "seed", &T::seed);
};

#endif