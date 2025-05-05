#include "request_generator.hpp"

#include "catch2/catch_test_macros.hpp"

using namespace palloc;

TEST_CASE("Base case - [Request Generator]") {
    constexpr double requestRate = 10;
    constexpr int maxTimeTillArrival = 5;
    constexpr int maxRequestDuration = 10;
    constexpr int dropoffNodes = 3;

    RequestGenerator generator({.dropoffNodes = dropoffNodes,
                                .maxTimeTillArrival = maxTimeTillArrival,
                                .maxRequestDuration = maxRequestDuration,
                                .seed = 1,
                                .requestRate = requestRate});

    constexpr Uint testRequestAmount = 1000;
    for (Uint i = 0; i < testRequestAmount; ++i) {
        const auto newRequests = generator.generate(i);
        for (const Request &request : newRequests) {
            REQUIRE(request.getArrival() <= maxTimeTillArrival);
            REQUIRE(request.getDropoffNode() <= dropoffNodes);
            REQUIRE(request.getRequestDuration() <= maxRequestDuration);
        }
    }
}