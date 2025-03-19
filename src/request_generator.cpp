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
        requests.emplace_back(dropoffDist(rng), getDuration());
    }

    return requests;
}

uint64_t RequestGenerator::getPoissonUpperBound(double rate) const {
    constexpr double rateThreshold = 100;
    constexpr double rateThresholdStddev = 10;
    constexpr double numStddevs = 3.0;
    constexpr double defaultUpperBound = rateThreshold + numStddevs * rateThresholdStddev;
    return (rate > rateThreshold) ? std::ceil(rate + numStddevs * std::sqrt(rate))
                                  : defaultUpperBound;
}

double RequestGenerator::getTimeMultiplier(uint64_t currentTimeOfDay) const {
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

uint64_t RequestGenerator::getDuration() {
    const uint64_t selectedBucket = durationDist(rng);

    const uint64_t start = durationBuckets[selectedBucket][0];
    const uint64_t end = std::min(durationBuckets[selectedBucket][1], maxRequestDuration);

    std::uniform_int_distribution<uint64_t> uniformDist(start, end);

    return uniformDist(rng);
}

std::vector<double> RequestGenerator::getDurationBuckets(uint64_t maxDuration) const {
    // Weights based on COWI
    constexpr std::array<double, 7> originalWeights{14.0, 13.0, 10.0, 16.0, 21.0, 9.0, 7.0};

    std::vector<double> weightBuckets;
    for (size_t i = 0; i < durationBuckets.size(); ++i) {
        const uint64_t start = durationBuckets[i][0];
        const uint64_t end = durationBuckets[i][1];
        
        if (start > maxDuration)  break;
    
        if (end <= maxDuration) { 
            // Full bucket
            weightBuckets.emplace_back(originalWeights[i]);
        } else { 
            // Partial bucket
            const double availableRange = maxDuration - start + 1;
            const double totalRange = end - start + 1;
            const double adjustedWeight = originalWeights[i] * (availableRange / totalRange);
            weightBuckets.emplace_back(adjustedWeight);;
        }
    }

    return weightBuckets;
}
