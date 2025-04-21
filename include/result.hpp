#ifndef RESULT_HPP
#define RESULT_HPP

#include <cstdint>
#include <vector>

#include "settings.hpp"
#include "trace.hpp"

namespace palloc {
class Result;  // forward

using Results = std::vector<Result>;

class Result {
   public:
    explicit Result(TraceLists traceLists, SimulatorSettings simSettings, size_t droppedRequests,
                    double globalAvgDuration, double globalAvgCost)
        : _traceLists(std::move(traceLists)),
          _simSettings(simSettings),
          _droppedRequests(droppedRequests),
          _globalAvgDuration(globalAvgDuration),
          _globalAvgCost(globalAvgCost) {}

    explicit Result(TraceList traceList, SimulatorSettings simSettings, size_t droppedRequests,
                    double globalAvgDuration, double globalAvgCost)
        : _traceLists{std::move(traceList)},
          _simSettings(simSettings),
          _droppedRequests(droppedRequests),
          _globalAvgDuration(globalAvgDuration),
          _globalAvgCost(globalAvgCost) {}

    explicit Result(const std::filesystem::path &inputPath) { loadResult(inputPath); };

    static Result aggregateResults(const Results &results);

    void saveToFile(const std::filesystem::path &outputPath, bool prettify) const;
    void loadResult(const std::filesystem::path &inputPath);

    TraceLists getTraceLists() const noexcept;
    SimulatorSettings getSimSettings() const noexcept;
    size_t getDroppedRequests() const noexcept;
    double getGlobalAvgDuration() const noexcept;
    double getGlobalAvgCost() const noexcept;

   private:
    friend struct glz::meta<Result>;

    TraceLists _traceLists;
    SimulatorSettings _simSettings{};
    size_t _droppedRequests{};
    double _globalAvgDuration{};
    double _globalAvgCost{};
};

using Results = std::vector<Result>;
}  // namespace palloc

template <>
struct glz::meta<palloc::Result> {
    using T = palloc::Result;
    static constexpr auto value =
        glz::object("total_dropped_requests", &T::_droppedRequests, "global_avg_duration",
                    &T::_globalAvgDuration, "global_avg_cost", &T::_globalAvgCost, "settings",
                    &T::_simSettings, "traces", &T::_traceLists);
};

#endif