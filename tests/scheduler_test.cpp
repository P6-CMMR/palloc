#include "catch2/catch_test_macros.hpp"
#include "scheduler.hpp"
#include "request_generator.hpp"
#include "environment.hpp"

using namespace palloc;

TEST_CASE("Base case") {
    Environment env("/home/vind/P6/palloc/data.json");
    
    SECTION("Request being simulated") {
        Requests requests;

        requests.emplace_back(1, 5, 0);
        const auto batchResult = Scheduler::scheduleBatch(env, requests);

        REQUIRE(batchResult.simulations.size() == 1);
        REQUIRE(batchResult.unassignedRequests.size() == 0);
        REQUIRE(batchResult.earlyRequests.size() == 0);
    }

    SECTION("Request being waiting") {
        Requests requests;

        requests.emplace_back(1, 5, 1);
        const auto batchResult = Scheduler::scheduleBatch(env, requests);

        REQUIRE(batchResult.simulations.size() == 0);
        REQUIRE(batchResult.unassignedRequests.size() == 0);
        REQUIRE(batchResult.earlyRequests.size() == 1);
    }
    
    SECTION("Request being unassinged") {
        Requests requests;

        requests.emplace_back(1, 1, 0);
        const auto batchResult = Scheduler::scheduleBatch(env, requests);

        REQUIRE(batchResult.simulations.size() == 0);
        REQUIRE(batchResult.unassignedRequests.size() == 1);
        REQUIRE(batchResult.earlyRequests.size() == 0);
    }
}