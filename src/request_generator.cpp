#include "request_generator.hpp"

using namespace palloc;

Requests RequestGenerator::generate(uint32_t currentTimeOfDay) {
    const auto count = getCount(currentTimeOfDay);
    Requests requests;
    requests.reserve(count);
    for (uint32_t i = 0; i < count; ++i) {
        requests.emplace_back(getDropoff(), getDuration(), getArrival());
    }

    return requests;
}

uint32_t RequestGenerator::getCount(uint32_t currentTimeOfDay) {
    const auto multiplier = getTimeMultiplier(currentTimeOfDay);
    const double adjustedRate = _requestRate * multiplier;
    std::poisson_distribution<uint32_t> requestCountDist(adjustedRate);
    return requestCountDist(_rng);
}

size_t RequestGenerator::getDropoff() { return _dropoffDist(_rng); }

uint32_t RequestGenerator::getArrival() { return _arrivalDist(_rng); }

uint32_t RequestGenerator::getDuration() {
    const uint32_t selectedBucket = _durationDist(_rng);
    const uint32_t start = DURATION_BUCKETS[selectedBucket][0];
    const uint32_t end = std::min(DURATION_BUCKETS[selectedBucket][1], _maxRequestDuration);

    std::uniform_int_distribution<uint32_t> uniformDist(start, end);

    return uniformDist(_rng);
}

double RequestGenerator::getTimeMultiplier(uint32_t currentTimeOfDay) {
    double timeInHours = static_cast<double>(currentTimeOfDay % 1440) / 60.0;

    constexpr double baseline = 0.4;

    // Morning peak parameters
    constexpr double morningAmplitude = 0.95;
    constexpr double morningCenter = 9.0;
    constexpr double morningWidth = 3.0;

    constexpr double morningFactor = 1.0 / (2.0 * morningWidth * morningWidth);
    constexpr double morningScale = morningAmplitude - baseline;

    // Evening peak parameters
    constexpr double eveningAmplitude = 0.9;
    constexpr double eveningCenter = 17.0;
    constexpr double eveningWidth = 3.0;

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

DoubleVector RequestGenerator::getDurationBuckets(uint32_t maxDuration) {
    DoubleVector weightBuckets;
    for (size_t i = 0; i < DURATION_BUCKETS.size(); ++i) {
        const uint32_t start = DURATION_BUCKETS[i][0];
        const uint32_t end = DURATION_BUCKETS[i][1];

        if (start > maxDuration) {
            break;
        }

        if (end <= maxDuration) {
            // Full bucket
            weightBuckets.push_back(originalWeights[i]);
        } else {
            // Partial bucket
            const double availableRange = static_cast<double>(maxDuration - start + 1);
            const double totalRange = static_cast<double>(end - start + 1);
            const double adjustedWeight = originalWeights[i] * (availableRange / totalRange);
            weightBuckets.push_back(adjustedWeight);
        }
    }

    return weightBuckets;
}
