#include "catch2/catch_test_macros.hpp"
#include "request_generator.hpp"

using namespace palloc;

TEST_CASE("Base case - [Request Generator]") {

    const int requestRate = 10;
    const int maxTimeTillArrival = 5;
    const int maxRequestDuration = 10;
    const int dropoffNodes = 3;

    RequestGenerator generator({
        .dropoffNodes = dropoffNodes,
        .maxTimeTillArrival = maxTimeTillArrival,
        .maxRequestDuration = maxRequestDuration,
        .seed = 1,
        .requestRate = requestRate 
    });
    const size_t testRequestAmount = 1000;
    const auto minRate = std::min(requestRate, 100);
    const auto upperBound =  minRate + 3 * sqrt(minRate);

    for (size_t i = 0; i < testRequestAmount; ++i) {
        const auto newRequests = generator.generate(i);
        
        REQUIRE(newRequests.size() <= upperBound);
        for (Request request : newRequests) {
            REQUIRE(request.getArrival() <= maxTimeTillArrival);
            REQUIRE(request.getDropoffNode() <= dropoffNodes);
            REQUIRE(request.getRequestDuration() <= maxRequestDuration);
        }
    }
}