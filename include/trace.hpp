#ifndef TRACE_HPP
#define TRACE_HPP

#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <list>
#include <ostream>
#include <vector>

#include "environment.hpp"
#include "glaze/glaze.hpp"

namespace palloc {
class Assignment {
   public:
    explicit Assignment() {}
    explicit Assignment(Coordinate dropoffCoordinate, Coordinate parkingCoordinate,
                        uint64_t requestDuration, uint64_t routeDuration)
        : _dropoffCoordinate(dropoffCoordinate),
          _parkingCoordinate(parkingCoordinate),
          _requestDuration(requestDuration),
          _routeDuration(routeDuration) {}

    uint32_t getRequestDuration() const noexcept;
   private:
    friend struct glz::meta<Assignment>;

    Coordinate _dropoffCoordinate;
    Coordinate _parkingCoordinate;

    uint64_t _requestDuration;
    uint64_t _routeDuration;
};

using Assignments = std::vector<Assignment>;

class Trace {
   public:
    explicit Trace() {}
    explicit Trace(uint64_t timestep, uint64_t currentTimeOfDay, size_t numberOfRequests,
                   size_t numberOfOngoingSimulations, uint64_t availableParkingSpots, double cost,
                   double averageDuration, size_t droppedRequests, 
                   size_t earlyRequests, Assignments assignments)
        : _assignments(std::move(assignments)),
          _numberOfRequests(numberOfRequests),
          _numberOfOngoingSimulations(numberOfOngoingSimulations),
          _availableParkingSpots(availableParkingSpots),
          _droppedRequests(droppedRequests),
          _earlyRequests(earlyRequests),
          _timestep(timestep),
          _currentTimeOfDay(currentTimeOfDay),
          _cost(cost),
          _averageDuration(averageDuration) {}

    size_t getNumberOfOngoingSimulations() const noexcept;
    size_t getDroppedRequests() const noexcept;
    size_t getEarlyRequests() const noexcept;
    size_t getNumberOfRequests() const noexcept;

    uint64_t getAvailableParkingSpots() const noexcept;

    uint64_t getTimeStep() const noexcept;

    double getCost() const noexcept;
    double getAverageDuration() const noexcept;

    Assignments getAssignments() const noexcept;

   private:
    friend struct glz::meta<Trace>;

    Assignments _assignments;

    size_t _numberOfRequests;
    size_t _numberOfOngoingSimulations;
    size_t _availableParkingSpots;
    size_t _droppedRequests;
    size_t _earlyRequests;

    uint64_t _timestep;
    uint64_t _currentTimeOfDay;

    double _cost;
    double _averageDuration;
};

std::ostream &operator<<(std::ostream &os, const Trace &trace);

using TraceList = std::list<Trace>;
using TraceLists = std::vector<TraceList>;
}  // namespace palloc

template <>
struct glz::meta<palloc::Assignment> {
    using T = palloc::Assignment;
    static constexpr auto value = glz::object(
        "dropoff_coordinate", &T::_dropoffCoordinate, "parking_coordinate", &T::_parkingCoordinate,
        "request_duration", &T::_requestDuration, "route_duration", &T::_routeDuration);
};

template <>
struct glz::meta<palloc::Trace> {
    using T = palloc::Trace;
    static constexpr auto value = glz::object(
        "timestep", &T::_timestep, "current_time_of_day", &T::_currentTimeOfDay,
        "number_of_requests", &T::_numberOfRequests, "number_of_ongoing_simulations",
        &T::_numberOfOngoingSimulations, "available_parking_spots", &T::_availableParkingSpots,
        "cost", &T::_cost, "average_duration", &T::_averageDuration, "dropped_requests",
        &T::_droppedRequests, "early_requests", &T::_earlyRequests, "assignments", &T::_assignments);
};

#endif