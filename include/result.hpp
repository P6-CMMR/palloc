#ifndef RESULT_HPP
#define RESULT_HPP

#include <cstdint>

#include "simulator.hpp"
#include "trace.hpp"

namespace palloc {
class Result {
   public:
    explicit Result(Traces traces, SimulatorSettings simSettings, size_t droppedRequests,
                    double globalAvgDuration, double globalAvgCost)
        : traces(std::move(traces)),
          simSettings(simSettings),
          droppedRequests(droppedRequests),
          globalAvgDuration(globalAvgDuration),
          globalAvgCost(globalAvgCost) {}

    void saveToFile(const std::filesystem::path &outputPath, bool prettify) const;

   private:
    friend struct glz::meta<Result>;

    Traces traces;
    SimulatorSettings simSettings;
    size_t droppedRequests;
    double globalAvgDuration;
    double globalAvgCost;
};
}  // namespace palloc

template <>
struct glz::meta<palloc::Result> {
    using T = palloc::Result;
    static constexpr auto value = glz::object(
        "total_dropped_requests", &T::droppedRequests, "global_avg_duration", &T::globalAvgDuration,
        "global_avg_cost", &T::globalAvgCost, "settings", &T::simSettings, "traces", &T::traces);
};

#endif