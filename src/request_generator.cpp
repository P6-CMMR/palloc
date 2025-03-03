#include "request_generator.hpp"

using namespace palloc;

const RequestGenerator::Requests RequestGenerator::generate() {
    const auto count = requestCountDist(rng);
    Requests requests;
    requests.reserve(count);
    for (uint64_t i = 0; i < count; ++i) {
        requests.emplace_back(Request{ .dropoffNode = dropoffDist(rng), .duration = durationDist(rng) });
    }

    return requests;
}
