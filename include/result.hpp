#ifndef RESULT_HPP
#define RESULT_HPP

#include <utility>
#include <vector>

#include "settings.hpp"
#include "trace.hpp"
#include "types.hpp"

namespace palloc {
class Result;  // forward

using Results = std::vector<Result>;

class Result {
   public:
    explicit Result(TraceList traceList, SimulatorSettings simSettings, size_t droppedRequests,
                    double globalAvgDuration, double globalAvgCost, Uint requestsGenerated,
                    size_t requestsScheduled, size_t requestsUnassigned, size_t processedRequests)
        : _traceList(std::move(traceList)),
          _simSettings(std::move(simSettings)),
          _droppedRequests(droppedRequests),
          _globalAvgDuration(globalAvgDuration),
          _globalAvgCost(globalAvgCost),
          _requestsGenerated(requestsGenerated),
          _requestsScheduled(requestsScheduled),
          _requestsUnassigned(requestsUnassigned),
          _processedRequests(processedRequests) {}

    TraceList getTraceList() const noexcept;
    SimulatorSettings getSimSettings() const noexcept;
    double getDuration() const noexcept;
    double getCost() const noexcept;
    size_t getDroppedRequests() const noexcept;
    Uint getRequestsGenerated() const noexcept;
    size_t getRequestsScheduled() const noexcept;
    size_t getRequestsUnassigned() const noexcept;
    size_t getProcessedRequests() const noexcept;

   private:
    friend struct glz::meta<Result>;

    TraceList _traceList;
    SimulatorSettings _simSettings{};
    size_t _droppedRequests{};
    double _globalAvgDuration{};
    double _globalAvgCost{};
    Uint _requestsGenerated{};
    size_t _requestsScheduled{};
    size_t _requestsUnassigned{};
    size_t _processedRequests{};
};

using Results = std::vector<Result>;

class AggregatedResult {
   public:
    explicit AggregatedResult(const Results &results);
    explicit AggregatedResult(const Path &inputPath);

    TraceLists getTraceLists() const noexcept;
    double getAvgDuration() const noexcept;
    double getAvgCost() const noexcept;
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
    double _globalAvgDuration{};
    double _globalAvgCost{};
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
        "total_dropped_requests", &T::_droppedRequests, "global_avg_duration",
        &T::_globalAvgDuration, "global_avg_cost", &T::_globalAvgCost, "requests_generated",
        &T::_requestsGenerated, "requests_scheduled", &T::_requestsScheduled, "requests_unassigned",
        &T::_requestsUnassigned, "time_elapsed", &T::_timeElapsed, "settings", &T::_simSettings,
        "traces", &T::_traceLists);
};

#endif