#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <filesystem>

#include "glaze/glaze.hpp"
#include "types.hpp"

namespace palloc {
struct SimulatorSettings {
    Uint timesteps;
    Uint startTime;
    Uint maxRequestDuration;
    double requestRate;
    Uint maxTimeTillArrival;
    Uint minParkingTime;
    Uint batchInterval;
    Uint commitInterval;
    Uint seed;
    bool useWeightedParking;
    std::string randomGenerator;
};

struct OutputSettings {
    Path outputPath;
    Uint numberOfRunsToAggregate;
    bool prettify;
    bool outputTrace;
};

struct GeneralSettings {
    Uint numberOfThreads;
};
}  // namespace palloc

template <>
struct glz::meta<palloc::SimulatorSettings> {
    using T = palloc::SimulatorSettings;
    static constexpr auto value = glz::object(
        "timesteps", &T::timesteps, "start_time", &T::startTime, "max_request_duration",
        &T::maxRequestDuration, "max_request_arrival", &T::maxTimeTillArrival, "min_parking_time",
        &T::minParkingTime, "request_rate", &T::requestRate, "batch_interval", &T::batchInterval,
        "commit_interval", &T::commitInterval, "seed", &T::seed, "using_weighted_parking",
        &T::useWeightedParking, "random_generator", &T::randomGenerator);
};

#endif