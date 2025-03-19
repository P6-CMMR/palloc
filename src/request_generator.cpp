#include "request_generator.hpp"

#include <cassert>

using namespace palloc;

size_t Request::getDropoffNode() const noexcept { return dropoffNode; }

uint64_t Request::getRequestDuration() const noexcept { return requestDuration; }

uint64_t Request::getTimesDropped() const noexcept { return timesDropped; }

void Request::decrementDuration() noexcept { --requestDuration; }

void Request::incrementTimesDropped() noexcept { ++timesDropped; }

bool Request::isDead() const noexcept { return requestDuration == 0; }

Requests RequestGenerator::generate(uint64_t currentTimeOfDay) {
    const auto multiplier = getTimeMultiplier(currentTimeOfDay);
    const double adjustedRate = requestRate * multiplier;
    std::poisson_distribution<uint64_t> requestCountDist(adjustedRate);
    const auto count = std::min(getPoissonUpperBound(requestRate), requestCountDist(rng));

    Requests requests;
    requests.reserve(count);
    for (uint64_t i = 0; i < count; ++i) {
        requests.emplace_back(dropoffDist(rng), durationDist(rng));
    }

    return requests;
}

uint64_t RequestGenerator::getPoissonUpperBound(double rate) {
    constexpr double rateThreshold = 100;
    constexpr double rateThresholdStddev = 10;
    constexpr double numStddevs = 3.0;
    constexpr double defaultUpperBound = rateThreshold + numStddevs * rateThresholdStddev;
    return (rate > rateThreshold) ? std::ceil(rate + numStddevs * std::sqrt(rate))
                                  : defaultUpperBound;
}

double RequestGenerator::getTimeMultiplier(uint64_t currentTimeOfDay) {
    double timeInHours = static_cast<double>(currentTimeOfDay % 1440) / 60.0;

    constexpr double baseline = 0.3;

    // Morning peak parameters
    constexpr double morningAmplitude = 0.8;
    constexpr double morningCenter = 8.0;
    constexpr double morningWidth = 1.5;

    constexpr double morningFactor = 1.0 / (2.0 * morningWidth * morningWidth);
    constexpr double morningScale = morningAmplitude - baseline;

    // Evening peak parameters
    constexpr double eveningAmplitude = 1.0;
    constexpr double eveningCenter = 17.0;
    constexpr double eveningWidth = 2.0;

    constexpr double eveningFactor = 1.0 / (2.0 * eveningWidth * eveningWidth);
    constexpr double eveningScale = eveningAmplitude - baseline;

    double morningDiff = timeInHours - morningCenter;
    double morningTerm = morningScale * std::exp(-morningDiff * morningDiff * morningFactor);

    double eveningDiff = timeInHours - eveningCenter;
    double eveningTerm = eveningScale * std::exp(-eveningDiff * eveningDiff * eveningFactor);

    double multiplier = baseline + morningTerm + eveningTerm;

    multiplier = std::min(1.0, multiplier);

    return multiplier;
}
