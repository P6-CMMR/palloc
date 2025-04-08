#include "catch2/catch_test_macros.hpp"
#include "scheduler.hpp"
#include "request_generator.hpp"
#include "environment.hpp"

using namespace palloc;


/*
 * TODO
 * - Check multiple requests
 * - Check errors
 *
*/

TEST_CASE("Base case - [Scheduler]", "[Scheduler]") {
    Environment env(std::filesystem::path(PROJECT_ROOT) / "tests/test_data.json");
    
    SECTION("Request being simulated") {
        Requests requests;

        requests.emplace_back(0, 10, 0);
        const auto batchResult = Scheduler::scheduleBatch(env, requests);

        REQUIRE(batchResult.simulations.size() == 1);
        REQUIRE(batchResult.unassignedRequests.size() == 0);
        REQUIRE(batchResult.earlyRequests.size() == 0);
        REQUIRE(batchResult.cost == 2);
    }

    SECTION("Request being waiting") {
        Requests requests;

        requests.emplace_back(1, 5, 1);
        const auto batchResult = Scheduler::scheduleBatch(env, requests);

        REQUIRE(batchResult.simulations.size() == 0);
        REQUIRE(batchResult.unassignedRequests.size() == 0);
        REQUIRE(batchResult.earlyRequests.size() == 1);
        REQUIRE(batchResult.cost == 0);
    }
    
    SECTION("Request being unassinged") {
        Requests requests;

        requests.emplace_back(1, 1, 0);
        const auto batchResult = Scheduler::scheduleBatch(env, requests);

        REQUIRE(batchResult.simulations.size() == 0);
        REQUIRE(batchResult.unassignedRequests.size() == 1);
        REQUIRE(batchResult.earlyRequests.size() == 0);
        REQUIRE(batchResult.cost == Scheduler::UNASSIGNED_PENALTY);
    }
}



TEST_CASE("Multiple requests - [Scheduler]") {
    Environment env(std::filesystem::path(PROJECT_ROOT) / "tests/test_data.json");
    
    SECTION("Parking is filled") {
        Requests requests;
        size_t requestAmount = env.getNumberOfParkings() + 1;
        for (size_t i = 0; i < requestAmount; ++i) {
            requests.emplace_back(1, 7, 0);
        }
        const auto batchResult = Scheduler::scheduleBatch(env, requests);

        REQUIRE(batchResult.simulations.size() == requestAmount - 1);
        REQUIRE(batchResult.unassignedRequests.size() == 1);
        REQUIRE(batchResult.earlyRequests.size() == 0);
        REQUIRE(batchResult.cost >  Scheduler::UNASSIGNED_PENALTY);
        REQUIRE(batchResult.cost < 2 * Scheduler::UNASSIGNED_PENALTY);
    }

    SECTION("Multiple unassigned requests") {
        Requests requests;
        size_t requestAmount = env.getNumberOfParkings();
        for (size_t i = 0; i < requestAmount; ++i) {
            requests.emplace_back(1, 1, 0);
        }
        const auto batchResult = Scheduler::scheduleBatch(env, requests);

        REQUIRE(batchResult.simulations.size() == 0);
        REQUIRE(batchResult.unassignedRequests.size() == requestAmount);
        REQUIRE(batchResult.earlyRequests.size() == 0);
        REQUIRE(batchResult.cost == requestAmount * Scheduler::UNASSIGNED_PENALTY);
    }
}