#ifndef AGGREGATED_RESULT_HPP
#define AGGREGATED_RESULT_HPP

#include "result.hpp"

namespace palloc {
class AggregatedResult {
   public:
    explicit AggregatedResult(const Results &results);
    explicit AggregatedResult(const Path &inputPath);

    TraceLists getTraceLists() const noexcept;
    double getAvgDuration() const noexcept;
    double getAvgCost() const noexcept;
    double getAvgVariableCount() const noexcept;
    size_t getTotalDroppedRequests() const noexcept;
    size_t getTotalRequestsGenerated() const noexcept;
    size_t getTotalRequestsScheduled() const noexcept;

    void setTimeElapsed(Uint timeElapsed) noexcept;

    void saveToFile(const Path &outputPath, bool prettify) const;
    void loadResult(const Path &inputPath);

   private:
    friend struct glz::meta<AggregatedResult>;

    TraceLists _traceLists;
    SimulatorSettings _simSettings{};
    size_t _droppedRequests{};
    double _avgDuration{};
    double _avgCost{};
    double _avgVariableCount{};
    Uint _requestsGenerated{};
    size_t _requestsScheduled{};
    size_t _requestsUnassigned{};
    size_t _processedRequests{};
    Uint _timeElapsed{};
};
}  // namespace palloc

template <>
struct glz::meta<palloc::AggregatedResult> {
    using T = palloc::AggregatedResult;
    static constexpr auto value = glz::object(
        "total_dropped_requests", &T::_droppedRequests, "avg_duration", &T::_avgDuration,
        "avg_cost", &T::_avgCost, "avg_var_count", &T::_avgVariableCount, "requests_generated",
        &T::_requestsGenerated, "requests_scheduled", &T::_requestsScheduled, "requests_unassigned",
        &T::_requestsUnassigned, "time_elapsed", &T::_timeElapsed, "settings", &T::_simSettings,
        "traces", &T::_traceLists);
};

#endif