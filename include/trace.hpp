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
    explicit Assignment(Coordinate dropoffCoordinate, Coordinate parkingCoordinate,
                        uint64_t requestDuration, uint64_t routeDuration)
        : dropoffCoordinate(dropoffCoordinate),
          parkingCoordinate(parkingCoordinate),
          requestDuration(requestDuration),
          routeDuration(routeDuration) {}

   private:
    friend struct glz::meta<Assignment>;

    Coordinate dropoffCoordinate;
    Coordinate parkingCoordinate;

    uint64_t requestDuration;
    uint64_t routeDuration;
};

using Assignments = std::vector<Assignment>;

class Trace {
   public:
    explicit Trace(uint64_t timestep, size_t numberOfRequests, size_t numberOfOngoingSimulations,
                   uint64_t availableParkingSpots, double cost, double averageDuration,
                   size_t droppedRequests, Assignments assignments)
        : timestep(timestep),
          numberOfRequests(numberOfRequests),
          numberOfOngoingSimulations(numberOfOngoingSimulations),
          availableParkingSpots(availableParkingSpots),
          cost(cost),
          averageDuration(averageDuration),
          droppedRequests(droppedRequests),
          assignments(std::move(assignments)) {}

    size_t getNumberOfOngoingSimulations() const noexcept;
    size_t getDroppedRequests() const noexcept;
    size_t getNumberOfRequests() const noexcept;

    uint64_t getAvailableParkingSpots() const noexcept;
    uint64_t getTimeStep() const noexcept;

    double getCost() const noexcept;
    double getAverageDuration() const noexcept;

   private:
    friend struct glz::meta<Trace>;

    Assignments assignments;

    size_t numberOfRequests;
    size_t numberOfOngoingSimulations;
    size_t availableParkingSpots;
    size_t droppedRequests;

    uint64_t timestep;

    double cost;
    double averageDuration;
};

std::ostream &operator<<(std::ostream &os, const Trace &trace);

using Traces = std::list<Trace>;
}  // namespace palloc

template <>
struct glz::meta<palloc::Assignment> {
    using T = palloc::Assignment;
    static constexpr auto value = glz::object(
        "dropoff_coordinate", &T::dropoffCoordinate, "parking_coordinate", &T::parkingCoordinate,
        "request_duration", &T::requestDuration, "route_duration", &T::routeDuration);
};

template <>
struct glz::meta<palloc::Trace> {
    using T = palloc::Trace;
    static constexpr auto value = glz::object(
        "timestep", &T::timestep, "number_of_requests", &T::numberOfRequests,
        "number_of_ongoing_simulations", &T::numberOfOngoingSimulations, "available_parking_spots",
        &T::availableParkingSpots, "cost", &T::cost, "average_duration", &T::averageDuration,
        "dropped_requests", &T::droppedRequests, "assignments", &T::assignments);
};

#endif