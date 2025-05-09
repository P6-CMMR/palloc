#include "result.hpp"

#include "utils.hpp"

using namespace palloc;

Result Result::aggregateResults(const Results &results) {
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
        traceLists.push_back(result.getTraceLists()[0]);
        durationVec.push_back(result.getDuration());
        costVec.push_back(result.getCost());
        droppedRequests += result.getDroppedRequests();
        requestsGenerated += result.getRequestsGenerated();
        requestsScheduled += result.getRequestsScheduled();
<<<<<<< Updated upstream
        requestsUnassigned += requestsGenerated - requestsScheduled;
        avgDurationVec.push_back(result.getGlobalTotalDuration());
        avgCostVec.push_back(result.getGlobalAvgCost());
=======
        requestsUnassigned += result.getRequestsUnassigned();
        processedRequests += result.getProcessedRequests();
>>>>>>> Stashed changes
    }

    auto globalAvgDuration = utils::KahanSum(durationVec);
    auto globalAvgCost = utils::KahanSum(costVec);

<<<<<<< Updated upstream
    const size_t numResults = results.size();
    globalAvgDuration /= static_cast<double>(requestsScheduled);
    globalAvgCost /= static_cast<double>(numResults);
=======
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
>>>>>>> Stashed changes

    return Result(traceLists, simSettings, droppedRequests, globalAvgDuration, globalAvgCost,
                  requestsGenerated, requestsScheduled, requestsUnassigned);
}

void Result::saveToFile(const std::filesystem::path &outputPath, bool prettify) const {
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

void Result::loadResult(const std::filesystem::path &inputPath) {
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

TraceLists Result::getTraceLists() const noexcept { return _traceLists; }

SimulatorSettings Result::getSimSettings() const noexcept { return _simSettings; }

size_t Result::getDroppedRequests() const noexcept { return _droppedRequests; }

<<<<<<< Updated upstream
double Result::getGlobalTotalDuration() const noexcept { return _globalTotalDuration; }
=======
double Result::getDuration() const noexcept { return _globalTotalDuration; }
>>>>>>> Stashed changes

double Result::getCost() const noexcept { return _globalAvgCost; }

Uint Result::getRequestsGenerated() const noexcept { return _requestsGenerated; }

size_t Result::getRequestsScheduled() const noexcept { return _requestsScheduled; }

size_t Result::getRequestsUnassigned() const noexcept { return _requestsUnassigned; }

size_t Result::getProcessedRequests() const noexcept { return _processedRequests; }
