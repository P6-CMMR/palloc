#include "request_generator.hpp"

using namespace palloc;

std::generator<RequestGenerator::Request> RequestGenerator::generate() {
    const auto count = requestCountDist(rng);
    for (size_t i = 0; i < count; ++i) {
        co_yield Request{dropoffDist(rng), durationDist(rng)};
    }
}