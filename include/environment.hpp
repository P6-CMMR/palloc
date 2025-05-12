#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include <filesystem>
#include <vector>

#include "glaze/glaze.hpp"
#include "types.hpp"

namespace palloc {

struct Coordinate {
    double latitude;
    double longitude;
};

class Environment {
   public:
    using DurationMatrix = std::vector<UintVector>;
    using Coordinates = std::vector<Coordinate>;

    explicit Environment(const Path &environmentPath);

    const Environment::DurationMatrix &getDropoffToParking() const noexcept;
    const Environment::DurationMatrix &getParkingToDropoff() const noexcept;

    UintVector &getAvailableParkingSpots() noexcept;

    const UintVector &getSmallestRoundTrips() const noexcept;
    const DoubleVector &getParkingWeights() const noexcept;

    const Environment::Coordinates &getDropoffCoordinates() const noexcept;
    const Environment::Coordinates &getParkingCoordinates() const noexcept;

    size_t getNumberOfDropoffs() const noexcept;
    size_t getNumberOfParkings() const noexcept;

   private:
    void loadEnvironment(const Path &environmentPath);

    friend struct glz::meta<Environment>;

    DurationMatrix _dropoffToParking;
    DurationMatrix _parkingToDropoff;
    UintVector _availableParkingSpots;
    UintVector _smallestRoundTrips;
    DoubleVector _parkingWeights;
    Coordinates _dropoffCoords;
    Coordinates _parkingCoords;
};
}  // namespace palloc

template <>
struct glz::meta<palloc::Coordinate> {
    using T = palloc::Coordinate;
    static constexpr auto value = glz::object("lat", &T::latitude, "lon", &T::longitude);
};

template <>
struct glz::meta<palloc::Environment> {
    using T = palloc::Environment;
    static constexpr auto value = glz::object(
        "dropoff_to_parking", &T::_dropoffToParking, "parking_to_dropoff", &T::_parkingToDropoff,
        "parking_capacities", &T::_availableParkingSpots, "dropoff_coords", &T::_dropoffCoords,
        "parking_coords", &T::_parkingCoords, "smallest_round_trips", &T::_smallestRoundTrips,
        "parking_weights", &T::_parkingWeights);
};

#endif