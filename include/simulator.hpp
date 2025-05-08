#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <filesystem>
#include <list>
#include <mutex>

#include "environment.hpp"
#include "glaze/glaze.hpp"
#include "request_generator.hpp"
#include "result.hpp"
#include "settings.hpp"
#include "trace.hpp"
#include "types.hpp"

namespace palloc {
class Simulation {
   public:
    explicit Simulation(Uint dropoffNode, Uint parkingNode, Uint requestDuration,
                        Uint earlyTimeLeft, Uint routeDuration)
        : _dropoffNode(dropoffNode),
          _parkingNode(parkingNode),
          _requestDuration(requestDuration),
          _durationLeft(requestDuration),
          _earlyTimeLeft(earlyTimeLeft),
          _routeDuration(routeDuration) {}

    Uint getDropoffNode() const noexcept;
    Uint getParkingNode() const noexcept;
    Uint getRequestDuration() const noexcept;
    Uint getDurationLeft() const noexcept;
    Uint getRouteDuration() const noexcept;

    bool isInDropoff() const noexcept;
    bool hasVisitedParking() const noexcept;
    bool isEarly() const noexcept;
    bool isDead() const noexcept;

    void setIsInDropoff(bool inDropoff) noexcept;
    void setHasVisitedParking(bool visitedParking) noexcept;

    void decrementDuration() noexcept;
    void decrementEarlyArrival() noexcept;

   private:
    Uint _dropoffNode;
    Uint _parkingNode;
    Uint _requestDuration;
    Uint _durationLeft;
    Uint _earlyTimeLeft;
    Uint _routeDuration;

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
    static void simulateRun(Environment env, const SimulatorSettings &simSettings, const OutputSettings &outputSettings, Results &results,
                            std::mutex &resultsMutex, Uint runNumber);

    static void updateSimulations(Simulations &simulations, Environment &env);
    static void insertNewRequests(RequestGenerator &generator, Uint currentTimeOfDay,
                                  Requests &requests);
    static void removeDeadRequests(Requests &unassignedRequests);
    static void decrementArrivalTime(Requests &earlyRequests);
    static void seperateTooEarlyRequests(Requests &requests, Uint maxDuration,
                                         Requests &earlyRequests);
    static void cutImpossibleRequests(Requests &requests, const UintVector &smallestRoundTrips);
};
}  // namespace palloc

#endif