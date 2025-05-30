#include "scheduler.hpp"

#include "catch2/catch_test_macros.hpp"
#include "environment.hpp"
#include "request_generator.hpp"

using namespace palloc;

TEST_CASE("Base case - [Scheduler]", "[Scheduler]") {
    Environment env(Path(PROJECT_ROOT) / "tests/test_data.json");
    SimulatorSettings simSettings{
        .timesteps = 0,
        .startTime = 0,
        .maxRequestDuration = 0,
        .requestRate = 0,
        .maxTimeTillArrival = 0,
        .minParkingTime = 0,
        .batchInterval = 0,
        .commitInterval = 0,
        .seed = 0,
        .useWeightedParking = false,
        .randomGenerator = "pcg"
    };

    SECTION("Request being simulated") {
        Requests requests;

        requests.emplace_back(0, 10, 0);
        const auto batchResult = Scheduler::scheduleBatch(env, requests, simSettings);

        REQUIRE(batchResult.simulations.size() == 1);
        REQUIRE(batchResult.unassignedRequests.empty());
        REQUIRE(batchResult.earlyRequests.empty());
        REQUIRE(batchResult.totalCost == 2.0);
    }

    SECTION("Request being waiting") {
        Requests requests;

        requests.emplace_back(1, 5, 1);
        const auto batchResult = Scheduler::scheduleBatch(env, requests, simSettings);

        REQUIRE(batchResult.simulations.empty());
        REQUIRE(batchResult.unassignedRequests.empty());
        REQUIRE(batchResult.earlyRequests.size() == 1);
    }

    SECTION("Request being unassigned") {
        Requests requests;

        requests.emplace_back(1, 1, 0);
        const auto batchResult = Scheduler::scheduleBatch(env, requests, simSettings);

        REQUIRE(batchResult.simulations.empty());
        REQUIRE(batchResult.unassignedRequests.size() == 1);
        REQUIRE(batchResult.earlyRequests.empty());
        REQUIRE(batchResult.totalCost == Scheduler::UNASSIGNED_PENALTY);
    }
}

TEST_CASE("Multiple requests - [Scheduler]") {
    Environment env(Path(PROJECT_ROOT) / "tests/test_data.json");
    SimulatorSettings simSettings{
        .timesteps = 0,
        .startTime = 0,
        .maxRequestDuration = 0,
        .requestRate = 0,
        .maxTimeTillArrival = 0,
        .minParkingTime = 0,
        .batchInterval = 0,
        .commitInterval = 0,
        .seed = 0,
        .useWeightedParking = false,
        .randomGenerator = "pcg"
    };

    SECTION("Parking is filled") {
        Requests requests;
        size_t requestAmount = env.getNumberOfParkings() + 1;
        for (size_t i = 0; i < requestAmount; ++i) {
            requests.emplace_back(1, 7, 0);
        }

        const auto batchResult = Scheduler::scheduleBatch(env, requests, simSettings);

        REQUIRE(batchResult.simulations.size() == requestAmount - 1);
        REQUIRE(batchResult.unassignedRequests.size() == 1);
        REQUIRE(batchResult.earlyRequests.empty());
        REQUIRE(batchResult.totalCost > Scheduler::UNASSIGNED_PENALTY);
        REQUIRE(batchResult.totalCost < 2 * Scheduler::UNASSIGNED_PENALTY);
    }

    SECTION("Multiple unassigned requests") {
        Requests requests;
        size_t requestAmount = env.getNumberOfParkings();
        for (size_t i = 0; i < requestAmount; ++i) {
            requests.emplace_back(1, 1, 0);
        }

        const auto batchResult = Scheduler::scheduleBatch(env, requests, simSettings);

        REQUIRE(batchResult.simulations.empty());
        REQUIRE(batchResult.unassignedRequests.size() == requestAmount);
        REQUIRE(batchResult.earlyRequests.empty());
        REQUIRE(batchResult.totalCost ==
                static_cast<double>(requestAmount * Scheduler::UNASSIGNED_PENALTY));
    }
}