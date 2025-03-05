#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include <cstdint>
#include <filesystem>
#include <glaze/glaze.hpp>
#include <vector>

namespace palloc {
class Environment {
   public:
    using UintVector = std::vector<uint64_t>;
    using DurationMatrix = std::vector<std::vector<double>>;

    explicit Environment(const std::filesystem::path &environmentPath) {
        loadEnvironment(environmentPath);
    }

    const Environment::DurationMatrix &getDropoffToParking() const noexcept;
    const Environment::DurationMatrix &getParkingToDropoff() const noexcept;
    const UintVector &getParkingCapacities() const noexcept;
    const size_t getNumberOfDropoffs() const noexcept;
    const size_t getNumberOfParkings() const noexcept;

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
};
}  // namespace palloc

template <>
struct glz::meta<palloc::Environment::EnvironmentData> {
    using T = palloc::Environment::EnvironmentData;
    static constexpr auto value =
        glz::object("dropoff_to_parking", &T::dropoffToParking,
                    "parking_to_dropoff", &T::parkingToDropoff,
                    "parking_capacities", &T::parkingCapacities);
};

#endif