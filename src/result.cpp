#include "result.hpp"

#include "utils.hpp"

using namespace palloc;

TraceList Result::getTraceList() const noexcept { return _traceList; }

SimulatorSettings Result::getSimSettings() const noexcept { return _simSettings; }

size_t Result::getDroppedRequests() const noexcept { return _droppedRequests; }

double Result::getDuration() const noexcept { return _globalAvgDuration; }

double Result::getCost() const noexcept { return _globalAvgCost; }

Uint Result::getRequestsGenerated() const noexcept { return _requestsGenerated; }

size_t Result::getRequestsScheduled() const noexcept { return _requestsScheduled; }

size_t Result::getRequestsUnassigned() const noexcept { return _requestsUnassigned; }

size_t Result::getProcessedRequests() const noexcept { return _processedRequests; }
