#ifndef RESULT_HPP
#define RESULT_HPP

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
                    double totalRunDuration, double totalRunCost, size_t totalRunVariables,
                    Uint requestsGenerated, size_t requestsScheduled, size_t requestsUnassigned,
                    size_t processedRequests)
        : _traceList(std::move(traceList)),
          _simSettings(std::move(simSettings)),
          _droppedRequests(droppedRequests),
          _totalRunDuration(totalRunDuration),
          _totalRunCost(totalRunCost),
          _totalRunVariables(totalRunVariables),
          _requestsGenerated(requestsGenerated),
          _requestsScheduled(requestsScheduled),
          _requestsUnassigned(requestsUnassigned),
          _processedRequests(processedRequests) {}

    TraceList getTraceList() const noexcept;
    SimulatorSettings getSimSettings() const noexcept;
    double getTotalDuration() const noexcept;
    double getTotalCost() const noexcept;
    size_t getTotalRunVariables() const noexcept;
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
    double _totalRunDuration{};
    double _totalRunCost{};
    size_t _totalRunVariables{};
    Uint _requestsGenerated{};
    size_t _requestsScheduled{};
    size_t _requestsUnassigned{};
    size_t _processedRequests{};
};

using Results = std::vector<Result>;
}  // namespace palloc

#endif