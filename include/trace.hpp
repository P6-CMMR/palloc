#ifndef TRACE_HPP
#define TRACE_HPP

#include <list>
#include <vector>

#include "assignment.hpp"
#include "environment.hpp"
#include "glaze/glaze.hpp"
#include "types.hpp"

namespace palloc {
class Trace {
   public:
    explicit Trace() {}
    explicit Trace(Uint timestep, Uint currentTimeOfDay, size_t numberOfRequests,
                   size_t numberOfOngoingSimulations, Uint availableParkingSpots, double cost,
                   double averageDuration, size_t droppedRequests, size_t earlyRequests,
                   Assignments assignments)
        : _assignments(std::move(assignments)),
          _numberOfRequests(numberOfRequests),
          _numberOfOngoingSimulations(numberOfOngoingSimulations),
          _availableParkingSpots(availableParkingSpots),
          _droppedRequests(droppedRequests),
          _earlyRequests(earlyRequests),
          _timestep(timestep),
          _currentTimeOfDay(currentTimeOfDay),
          _averageCost(cost),
          _averageDuration(averageDuration) {}

    size_t getNumberOfOngoingSimulations() const noexcept;
    size_t getDroppedRequests() const noexcept;
    size_t getEarlyRequests() const noexcept;
    size_t getNumberOfRequests() const noexcept;

    Uint getAvailableParkingSpots() const noexcept;

    Uint getTimeStep() const noexcept;

    double getCost() const noexcept;
    double getAverageDuration() const noexcept;

    Assignments getAssignments() const noexcept;

   private:
    friend struct glz::meta<Trace>;

    Assignments _assignments;

    size_t _numberOfRequests{};
    size_t _numberOfOngoingSimulations{};
    Uint _availableParkingSpots{};
    size_t _droppedRequests{};
    size_t _earlyRequests{};

    Uint _timestep{};
    Uint _currentTimeOfDay{};

    double _averageCost{};
    double _averageDuration{};
};

using TraceList = std::list<Trace>;
using TraceLists = std::vector<TraceList>;
}  // namespace palloc

template <>
struct glz::meta<palloc::Trace> {
    using T = palloc::Trace;
    static constexpr auto value =
        glz::object("timestep", &T::_timestep, "current_time_of_day", &T::_currentTimeOfDay,
                    "number_of_requests", &T::_numberOfRequests, "number_of_ongoing_simulations",
                    &T::_numberOfOngoingSimulations, "available_parking_spots",
                    &T::_availableParkingSpots, "average_cost", &T::_averageCost, "average_duration",
                    &T::_averageDuration, "dropped_requests", &T::_droppedRequests,
                    "early_requests", &T::_earlyRequests, "assignments", &T::_assignments);
};

#endif