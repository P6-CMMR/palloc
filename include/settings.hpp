#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <cstdint>
#include <filesystem>

namespace palloc {
struct SimulatorSettings {
    uint64_t timesteps;
    uint64_t startTime;
    uint64_t maxRequestDuration;
    double requestRate;
    uint64_t maxTimeTillArrival;
    uint64_t batchInterval;
    uint64_t seed;
};

struct OutputSettings {
    std::filesystem::path path;
    uint64_t numberOfRunsToAggregate;
    bool prettify;
};

struct GeneralSettings {
    uint64_t numberOfThreads;
};
}  // namespace palloc

#endif