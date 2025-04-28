#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <cstdint>
#include <filesystem>

#include "glaze/glaze.hpp"

namespace palloc {
struct SimulatorSettings {
    uint32_t timesteps;
    uint32_t startTime;
    uint32_t maxRequestDuration;
    double requestRate;
    uint32_t maxTimeTillArrival;
    uint32_t minParkingTime;
    uint32_t batchInterval;
    uint32_t commitInterval;
    uint32_t seed;
    bool useWeightedParking;
};

struct OutputSettings {
    std::filesystem::path outputPath;
    uint32_t numberOfRunsToAggregate;
    bool prettify;
};

struct GeneralSettings {
    uint32_t numberOfThreads;
};
}  // namespace palloc

template <>
struct glz::meta<palloc::SimulatorSettings> {
    using T = palloc::SimulatorSettings;
    static constexpr auto value = glz::object(
        "timesteps", &T::timesteps, "start_time", &T::startTime, "max_request_duration",
        &T::maxRequestDuration, "max_request_arrival", &T::maxTimeTillArrival, "min_parking_time",
        &T::minParkingTime, "request_rate", &T::requestRate, "batch_interval", &T::batchInterval, 
        "commit_interval", &T::commitInterval, "using_weighted_parking", &T::useWeightedParking, "seed", &T::seed);
};

#endif