#ifndef ASSIGNMENT_HPP
#define ASSIGNMENT_HPP

#include <vector>

#include "environment.hpp"
#include "glaze/glaze.hpp"
#include "types.hpp"

namespace palloc {
class Assignment {
   public:
    explicit Assignment() {}
    explicit Assignment(Coordinate dropoffCoordinate, Coordinate parkingCoordinate,
                        Uint requestDuration, Uint routeDuration)
        : _dropoffCoordinate(dropoffCoordinate),
          _parkingCoordinate(parkingCoordinate),
          _requestDuration(requestDuration),
          _routeDuration(routeDuration) {}

    Uint getRequestDuration() const noexcept;

   private:
    friend struct glz::meta<Assignment>;

    Coordinate _dropoffCoordinate{};
    Coordinate _parkingCoordinate{};

    Uint _requestDuration{};
    Uint _routeDuration{};
};

using Assignments = std::vector<Assignment>;
}  // namespace palloc

template <>
struct glz::meta<palloc::Assignment> {
    using T = palloc::Assignment;
    static constexpr auto value = glz::object(
        "dropoff_coordinate", &T::_dropoffCoordinate, "parking_coordinate", &T::_parkingCoordinate,
        "request_duration", &T::_requestDuration, "route_duration", &T::_routeDuration);
};

#endif