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
        : _traces(std::move(traces)),
          _simSettings(simSettings),
          _droppedRequests(droppedRequests),
          _globalAvgDuration(globalAvgDuration),
          _globalAvgCost(globalAvgCost) {}

    void saveToFile(const std::filesystem::path &outputPath, bool prettify) const;

   private:
    friend struct glz::meta<Result>;

    Traces _traces;
    SimulatorSettings _simSettings;
    size_t _droppedRequests;
    double _globalAvgDuration;
    double _globalAvgCost;
};
}  // namespace palloc

template <>
struct glz::meta<palloc::Result> {
    using T = palloc::Result;
    static constexpr auto value =
        glz::object("total_dropped_requests", &T::_droppedRequests, "global_avg_duration",
                    &T::_globalAvgDuration, "global_avg_cost", &T::_globalAvgCost, "settings",
                    &T::_simSettings, "traces", &T::_traces);
};

#endif