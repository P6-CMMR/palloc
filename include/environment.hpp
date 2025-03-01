#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include <cstdint>
#include <filesystem>
#include <glaze/glaze.hpp>
#include <vector>

namespace palloc {
class Environment {
   public:
    using IntVector = std::vector<int64_t>;
    using DurationMatrix = std::vector<IntVector>;

    explicit Environment(const std::filesystem::path &environmentPath) {
        loadEnvironment(environmentPath);
    }

    const DurationMatrix &getDropoffToParking() const noexcept;
    const DurationMatrix &getParkingToDropoff() const noexcept;
    const IntVector &getParkingCapacities() const noexcept;

    struct EnvironmentData {
        DurationMatrix dropoffToParking;
        DurationMatrix parkingToDropoff;
        IntVector parkingCapacities;
    };

   private:
    void loadEnvironment(const std::filesystem::path &environmentPath);

    DurationMatrix dropoffToParking;
    DurationMatrix parkingToDropoff;
    IntVector parkingCapacities;
};
}  // namespace palloc

template <>
struct glz::meta<palloc::Environment::EnvironmentData> {
    using T = palloc::Environment::EnvironmentData;
    static constexpr auto value =
        glz::object("dropoff_to_parking", &T::dropoffToParking, "parking_to_dropoff",
                    &T::parkingToDropoff, "parking_capacities", &T::parkingCapacities);
};

#endif