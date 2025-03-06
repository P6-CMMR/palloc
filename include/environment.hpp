#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include <cstdint>
#include <filesystem>
#include <vector>

#include "glaze/glaze.hpp"

namespace palloc {
class Environment {
   public:
    using UintVector = std::vector<uint64_t>;
    using DurationMatrix = std::vector<UintVector>;

    explicit Environment(const std::filesystem::path &environmentPath) {
        loadEnvironment(environmentPath);
    }

    const Environment::DurationMatrix &getDropoffToParking() const noexcept;
    const Environment::DurationMatrix &getParkingToDropoff() const noexcept;

    UintVector &getAvailableParkingSpots() noexcept;

    size_t getNumberOfDropoffs() const noexcept;
    size_t getNumberOfParkings() const noexcept;

    struct EnvironmentData {
        DurationMatrix dropoffToParking;
        DurationMatrix parkingToDropoff;
        UintVector parkingCapacities;
    };

   private:
    void loadEnvironment(const std::filesystem::path &environmentPath);

    DurationMatrix dropoffToParking;
    DurationMatrix parkingToDropoff;
    UintVector parkingCapacities;
    UintVector availableParkingSpots;
};
}  // namespace palloc

template <>
struct glz::meta<palloc::Environment::EnvironmentData> {
    using T = palloc::Environment::EnvironmentData;
    constexpr static auto value =
        glz::object("dropoff_to_parking", &T::dropoffToParking, "parking_to_dropoff",
                    &T::parkingToDropoff, "parking_capacities", &T::parkingCapacities);
};

#endif