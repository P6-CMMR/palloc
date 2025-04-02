#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <cstdint>
#include <filesystem>
#include <list>
#include <mutex>

#include "environment.hpp"
#include "glaze/glaze.hpp"
#include "request_generator.hpp"
#include "result.hpp"
#include "settings.hpp"
#include "trace.hpp"

namespace palloc {
class Simulation {
   public:
    explicit Simulation(uint64_t dropoffNode, uint64_t parkingNode, uint64_t requestDuration,
                        uint64_t routeDuration)
        : _dropoffNode(dropoffNode),
          _parkingNode(parkingNode),
          _requestDuration(requestDuration),
          _durationLeft(requestDuration),
          _routeDuration(routeDuration) {}

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
    uint64_t _dropoffNode;
    uint64_t _parkingNode;
    uint64_t _requestDuration;
    uint64_t _durationLeft;
    uint64_t _routeDuration;

    bool _inDropoff{true};
    bool _visitedParking{false};
};

using Simulations = std::list<Simulation>;

class Simulator {
   public:
    static void simulate(Environment &env, const SimulatorSettings &simSettings,
                         const OutputSettings &outputSettings,
                         const GeneralSettings &generalSettings);

   private:
    static void simulateRun(Environment env, const SimulatorSettings &simSettings, Results &results,
                            std::mutex &resultsMutex, uint64_t runNumber);

    static void updateSimulations(Simulations &simulations, Environment &env);
    static void insertNewRequests(RequestGenerator &generator, uint64_t currentTimeOfDay,
                                  Requests &requests);
    static void removeDeadRequests(Requests &unassignedRequests);
    static void decrementArrivalTime(Requests &earlyRequests);
    static void seperateTooEarlyRequests(Requests &requests, uint64_t maxDuration,
                                         Requests &earlyRequests);
    static void cutImpossibleRequests(Requests &requests, const UintVector &smallestRoundTrips);
};
}  // namespace palloc

template <>
struct glz::meta<palloc::SimulatorSettings> {
    using T = palloc::SimulatorSettings;
    static constexpr auto value = glz::object(
        "timesteps", &T::timesteps, "start_time", &T::startTime, "max_request_duration",
        &T::maxRequestDuration, "max_time_till_arrival", &T::maxTimeTillArrival, "request_rate",
        &T::requestRate, "batch_interval", &T::batchInterval, "seed", &T::seed);
};

#endif