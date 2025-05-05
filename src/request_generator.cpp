#include "request_generator.hpp"

#include <cassert>

using namespace palloc;

Requests RequestGenerator::generate(Uint currentTimeOfDay) {
    const auto count = getCount(currentTimeOfDay);
    _requestsGenerated += count;

    Requests requests;
    requests.reserve(count);
    for (Uint i = 0; i < count; ++i) {
        requests.emplace_back(getDropoff(), getDuration(), getArrival());
    }

    return requests;
}

Uint RequestGenerator::getCount(Uint currentTimeOfDay) {
    const auto multiplier = getTimeMultiplier(currentTimeOfDay);
    const double adjustedRate = _requestRate * multiplier;
    std::poisson_distribution<Uint> requestCountDist(adjustedRate);
    return requestCountDist(_rng);
}

size_t RequestGenerator::getDropoff() { return _dropoffDist(_rng); }

Uint RequestGenerator::getArrival() { return _arrivalDist(_rng); }

Uint RequestGenerator::getDuration() {
    const Uint selectedBucket = _durationDist(_rng);
    const Uint start = DURATION_BUCKETS[selectedBucket][0];
    const Uint end = std::min(DURATION_BUCKETS[selectedBucket][1], _maxRequestDuration);

    std::uniform_int_distribution<Uint> uniformDist(start, end);

    return uniformDist(_rng);
}

Uint RequestGenerator::getRequestsGenerated() const noexcept { return _requestsGenerated; }

double RequestGenerator::getTimeMultiplier(Uint currentTimeOfDay) {
    const Uint hour = currentTimeOfDay / 60;
    assert(hour < 24);
    return TRAFFIC_WEIGHTS[hour];
}

DoubleVector RequestGenerator::getDurationBuckets(Uint maxDuration) {
    DoubleVector weightBuckets;
    for (size_t i = 0; i < DURATION_BUCKETS.size(); ++i) {
        const Uint start = DURATION_BUCKETS[i][0];
        const Uint end = DURATION_BUCKETS[i][1];

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
