#include "result.hpp"

#include "utils.hpp"

using namespace palloc;

Result Result::aggregateResults(const Results &results) {
    TraceLists traceLists;
    SimulatorSettings simSettings = results[0].getSimSettings();
    size_t droppedRequests = 0;
    Uint requestsGenerated = 0;
    size_t requestsScheduled = 0;

    std::vector<double> avgDurationVec;
    avgDurationVec.reserve(results.size());

    std::vector<double> avgCostVec;
    avgCostVec.reserve(results.size());
    for (const Result &result : results) {
        traceLists.push_back(result.getTraceLists()[0]);
        droppedRequests += result.getDroppedRequests();
        requestsGenerated += result.getRequestsGenerated();
        requestsScheduled += result.getRequestsScheduled();
        avgDurationVec.push_back(result.getGlobalAvgDuration());
        avgCostVec.push_back(result.getGlobalAvgCost());
    }

    auto globalAvgDuration = utils::KahanSum(avgDurationVec);
    auto globalAvgCost = utils::KahanSum(avgCostVec);

    const size_t numResults = results.size();
    globalAvgDuration /= static_cast<double>(numResults);
    globalAvgCost /= static_cast<double>(numResults);

    return Result(traceLists, simSettings, droppedRequests, globalAvgDuration, globalAvgCost,
                  requestsGenerated, requestsScheduled);
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

double Result::getGlobalAvgDuration() const noexcept { return _globalAvgDuration; }

double Result::getGlobalAvgCost() const noexcept { return _globalAvgCost; }

Uint Result::getRequestsGenerated() const noexcept { return _requestsGenerated; }

size_t Result::getRequestsScheduled() const noexcept { return _requestsScheduled; }
