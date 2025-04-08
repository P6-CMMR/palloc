#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <cstdint>
#include <filesystem>

namespace palloc {
struct SimulatorSettings {
    uint32_t timesteps;
    uint32_t startTime;
    uint32_t maxRequestDuration;
    double requestRate;
    uint32_t maxTimeTillArrival;
    uint32_t batchInterval;
    uint32_t seed;
};

struct OutputSettings {
    std::filesystem::path path;
    uint32_t numberOfRunsToAggregate;
    bool prettify;
};

struct GeneralSettings {
    uint32_t numberOfThreads;
};
}  // namespace palloc

#endif