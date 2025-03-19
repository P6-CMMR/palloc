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
    const auto count = requestCountDist(rng) * multiplier;
    Requests requests;
    requests.reserve(count);
    for (uint64_t i = 0; i < count; ++i) {
        requests.emplace_back(dropoffDist(rng), durationDist(rng));
    }

    return requests;
}

double RequestGenerator::getTimeMultiplier(uint64_t currentTimeOfDay) {
    double multiplier = 1;
    // TODO: add more advanced logic
    assert(multiplier <= 1);
    return multiplier;
}
