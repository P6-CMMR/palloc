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

    const DurationMatrix &getDurationMatrix() const noexcept;
    const IntVector &getParkingCapacities() const noexcept;
    const uint64_t getNumberOfParkings() const noexcept;
    const uint64_t getNumberOfDropoffs() const noexcept;

    struct EnvironmentData {
        DurationMatrix durationMatrix;
        IntVector parkingCapacities;
        uint64_t numberOfParkings;
        uint64_t numberOfDropoffs;
    };

   private:
    void loadEnvironment(const std::filesystem::path &environmentPath);

    DurationMatrix durationMatrix;
    IntVector parkingCapacities;
    uint64_t numberOfParkings;
    uint64_t numberOfDropoffs;
};
}  // namespace palloc

template <>
struct glz::meta<palloc::Environment::EnvironmentData> {
    using T = palloc::Environment::EnvironmentData;
    static constexpr auto value =
        glz::object("duration_matrix", &T::durationMatrix, 
                    "parking_capacities", &T::parkingCapacities, 
                    "num_parkings", &T::numberOfParkings,
                    "num_dropoffs", &T::numberOfDropoffs);
};

#endif