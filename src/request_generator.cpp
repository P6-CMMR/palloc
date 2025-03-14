#include "request_generator.hpp"

using namespace palloc;

size_t Request::getDropoffNode() const noexcept { return dropoffNode; }

uint64_t Request::getDuration() const noexcept { return duration; }

uint64_t Request::getTimesDropped() const noexcept { return timesDropped; }

void Request::decrementDuration() noexcept { --duration; }

void Request::incrementTimesDropped() noexcept { ++timesDropped; }

bool Request::isDead() const noexcept { return duration == 0; }

Requests RequestGenerator::generate() {
    const auto count = requestCountDist(rng);
    Requests requests;
    requests.reserve(count);
    for (uint64_t i = 0; i < count; ++i) {
        requests.emplace_back(dropoffDist(rng), durationDist(rng));
    }

    return requests;
}
