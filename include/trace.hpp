#ifndef TRACE_HPP
#define TRACE_HPP

#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <list>
#include <ostream>

#include "glaze/glaze.hpp"

namespace palloc {
class Trace {
   public:
    explicit Trace(uint64_t timestep, size_t numberOfRequests, size_t numberOfOngoingSimulations,
                   uint64_t availableParkingSpots, double cost, double averageDuration,
                   size_t droppedRequests)
        : timestep(timestep),
          numberOfRequests(numberOfRequests),
          numberOfOngoingSimulations(numberOfOngoingSimulations),
          availableParkingSpots(availableParkingSpots),
          cost(cost),
          averageDuration(averageDuration),
          droppedRequests(droppedRequests) {}

    uint64_t getTimeStep() const noexcept;
    size_t getNumberOfRequests() const noexcept;
    size_t getNumberOfOngoingSimulations() const noexcept;
    uint64_t getAvailableParkingSpots() const noexcept;

    double getCost() const noexcept;
    double getAverageDuration() const noexcept;

    size_t getDroppedRequests() const noexcept;

   private:
    friend struct glz::meta<Trace>;

    uint64_t timestep;
    size_t numberOfRequests;
    size_t numberOfOngoingSimulations;
    size_t availableParkingSpots;
    double cost;
    double averageDuration;
    size_t droppedRequests;
};

std::ostream &operator<<(std::ostream &os, const Trace &trace);

using Traces = std::list<Trace>;
}  // namespace palloc

template <>
struct glz::meta<palloc::Trace> {
    using T = palloc::Trace;
    static constexpr auto value = glz::object(
        "timestep", &T::timestep, "number_of_requests", &T::numberOfRequests,
        "number_of_ongoing_simulations", &T::numberOfOngoingSimulations, "available_parking_spots",
        &T::availableParkingSpots, "cost", &T::cost, "average_duration", &T::averageDuration,
        "dropped_requests", &T::droppedRequests);
};

#endif