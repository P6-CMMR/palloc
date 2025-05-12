#include "aggregated_result.hpp"

#include "utils.hpp"

using namespace palloc;

AggregatedResult::AggregatedResult(const Results &results) {
    TraceLists traceLists;
    traceLists.reserve(results.size());

    SimulatorSettings simSettings = results[0].getSimSettings();
    size_t droppedRequests = 0;
    Uint requestsGenerated = 0;
    size_t requestsScheduled = 0;
    size_t requestsUnassigned = 0;
    size_t processedRequests = 0;

    DoubleVector durationVec;
    durationVec.reserve(results.size());

    DoubleVector costVec;
    costVec.reserve(results.size());
    for (const Result &result : results) {
        traceLists.push_back(result.getTraceList());
        durationVec.push_back(result.getDuration());
        costVec.push_back(result.getCost());
        droppedRequests += result.getDroppedRequests();
        requestsGenerated += result.getRequestsGenerated();
        requestsScheduled += result.getRequestsScheduled();
        requestsUnassigned += result.getRequestsUnassigned();
        processedRequests += result.getProcessedRequests();
    }

    auto globalAvgDuration = utils::KahanSum(durationVec);
    auto globalAvgCost = utils::KahanSum(costVec);
    if (requestsScheduled == 0) {
        globalAvgDuration = 0.0;
    } else {
        globalAvgDuration /= static_cast<double>(requestsScheduled);
    }

    if (processedRequests == 0) {
        globalAvgCost = 0.0;
    } else {
        globalAvgCost /= static_cast<double>(processedRequests);
    }

    _traceLists = traceLists;
    _simSettings = simSettings;
    _droppedRequests = droppedRequests;
    _globalAvgDuration = globalAvgDuration;
    _globalAvgCost = globalAvgCost;
    _requestsGenerated = requestsGenerated;
    _requestsScheduled = requestsScheduled;
    _requestsUnassigned = requestsUnassigned;
    _processedRequests = processedRequests;
}

AggregatedResult::AggregatedResult(const Path &inputPath) { loadResult(inputPath); }

TraceLists AggregatedResult::getTraceLists() const noexcept { return _traceLists; }

double AggregatedResult::getAvgDuration() const noexcept { return _globalAvgDuration; }

double AggregatedResult::getAvgCost() const noexcept { return _globalAvgCost; }

size_t AggregatedResult::getTotalDroppedRequests() const noexcept { return _droppedRequests; }

size_t AggregatedResult::getTotalRequestsGenerated() const noexcept { return _requestsGenerated; }

size_t AggregatedResult::getTotalRequestsScheduled() const noexcept { return _requestsScheduled; }

void AggregatedResult::setTimeElapsed(Uint timeElapsed) noexcept { _timeElapsed = timeElapsed; }

void AggregatedResult::saveToFile(const Path &outputPath, bool prettify) const {
    // write_file_json evaluated at compile time so we need both cases
    glz::error_ctx error;
    if (prettify) {
        error = glz::write_file_json<glz::opts{.prettify = true, .indentation_width = 4}>(
            *this, outputPath.string(), std::string{});
    } else {
        error = glz::write_file_json<>(*this, outputPath.string(), std::string{});
    }

    if (error) {
        const auto errorStr = glz::format_error(error, std::string{});
        throw std::runtime_error("Failed to write to: " + outputPath.string() +
                                 "\nwith error: " + errorStr);
    }
}

void AggregatedResult::loadResult(const Path &inputPath) {
    if (!std::filesystem::exists(inputPath)) {
        throw std::runtime_error("Result file does not exist: " + inputPath.string());
    }

    const auto error = glz::read_file_json(*this, inputPath.string(), std::string{});
    if (error) {
        const auto errorStr = glz::format_error(error, std::string{});
        throw std::runtime_error("Failed to read result file: " + inputPath.string() +
                                 "\nwith error: " + errorStr);
    }
}
