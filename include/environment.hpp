#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include <array>
#include <cstdint>
#include <filesystem>
#include <vector>

#include "glaze/glaze.hpp"

namespace palloc {

struct Coordinate {
    double longitude;
    double latitude;
};

class Environment {
   public:
    using UintVector = std::vector<uint64_t>;
    using DurationMatrix = std::vector<UintVector>;
    using Coordinates = std::vector<Coordinate>;

    explicit Environment(const std::filesystem::path &environmentPath) {
        loadEnvironment(environmentPath);
    }

    const Environment::DurationMatrix &getDropoffToParking() const noexcept;
    const Environment::DurationMatrix &getParkingToDropoff() const noexcept;

    UintVector &getAvailableParkingSpots() noexcept;

    const Environment::Coordinates &getDropoffCoordinates() const noexcept;
    const Environment::Coordinates &getParkingCoordinates() const noexcept;

    size_t getNumberOfDropoffs() const noexcept;
    size_t getNumberOfParkings() const noexcept;

   private:
    void loadEnvironment(const std::filesystem::path &environmentPath);

    friend struct glz::meta<Environment>;

    DurationMatrix dropoffToParking;
    DurationMatrix parkingToDropoff;
    UintVector availableParkingSpots;
    Coordinates dropoffCoords;
    Coordinates parkingCoords;
};
}  // namespace palloc

template <>
struct glz::meta<palloc::Coordinate> {
    using T = palloc::Coordinate;
    static constexpr auto value = glz::array(&T::latitude, &T::longitude);
};

template <>
struct glz::meta<palloc::Environment> {
    using T = palloc::Environment;
    static constexpr auto value =
        glz::object("dropoff_to_parking", &T::dropoffToParking, "parking_to_dropoff",
                    &T::parkingToDropoff, "parking_capacities", &T::availableParkingSpots,
                    "dropoff_coords", &T::dropoffCoords, "parking_coords", &T::parkingCoords);
};

#endif