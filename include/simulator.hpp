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
    explicit Simulation(uint32_t dropoffNode, uint32_t parkingNode, uint32_t requestDuration,
                        uint32_t routeDuration)
        : _dropoffNode(dropoffNode),
          _parkingNode(parkingNode),
          _requestDuration(requestDuration),
          _durationLeft(requestDuration),
          _routeDuration(routeDuration) {}

    uint32_t getDropoffNode() const noexcept;
    uint32_t getParkingNode() const noexcept;
    uint32_t getRequestDuration() const noexcept;
    uint32_t getDurationLeft() const noexcept;
    uint32_t getRouteDuration() const noexcept;

    bool isInDropoff() const noexcept;
    bool hasVisitedParking() const noexcept;

    void setIsInDropoff(bool inDropoff) noexcept;
    void setHasVisitedParking(bool visitedParking) noexcept;

    void decrementDuration() noexcept;

   private:
    uint32_t _dropoffNode;
    uint32_t _parkingNode;
    uint32_t _requestDuration;
    uint32_t _durationLeft;
    uint32_t _routeDuration;

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
                            std::mutex &resultsMutex, uint32_t runNumber);

    static void updateSimulations(Simulations &simulations, Environment &env);
    static void insertNewRequests(RequestGenerator &generator, uint32_t currentTimeOfDay,
                                  Requests &requests);
    static void removeDeadRequests(Requests &unassignedRequests);
    static void decrementArrivalTime(Requests &earlyRequests);
    static void seperateTooEarlyRequests(Requests &requests, uint32_t maxDuration,
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