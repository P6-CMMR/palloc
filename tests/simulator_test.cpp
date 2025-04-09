#include "catch2/catch_test_macros.hpp"
#include "simulator.hpp"

using namespace palloc;

TEST_CASE("Base case - [Simulator]") {
    const std::filesystem::path testDataPath = std::filesystem::path(PROJECT_ROOT) / "tests/test_data.json";
    const std::filesystem::path tempResultPath = std::filesystem::path(PROJECT_ROOT) / "tests/temp_result.json";

    OutputSettings outputSettings{.path = tempResultPath, .numberOfRunsToAggregate = 1, .prettify = true};
    GeneralSettings generalSettings{.numberOfThreads = 1};

    const auto timesteps = 1000;
    const auto maxRequestDuration = 5;
    const auto requestRate = 10;
    const auto maxTimeTillArrival = 5;
    const auto batchInterval = 2;
    const auto seed = 1;
    SimulatorSettings simSettings{
        .timesteps = timesteps,
        .maxRequestDuration = maxRequestDuration,
        .requestRate = requestRate,
        .maxTimeTillArrival = maxTimeTillArrival,
        .batchInterval = batchInterval,
        .seed = seed
    };

    Environment env(testDataPath);

    Simulator::simulate(env, simSettings, outputSettings, generalSettings);

    Result result(tempResultPath);

    const auto traces = result.getTraceLists()[0];
    REQUIRE(traces.size() == timesteps);

    
    Trace earlierTrace = traces.front();
    std::vector<std::pair<uint32_t, Assignment>> assignements;
    
    const uint32_t parkingAmount = earlierTrace.getAvailableParkingSpots() + earlierTrace.getNumberOfOngoingSimulations();
    size_t tempDropAmount = 0;

    for (auto trace : traces) {
        if (trace.getTimeStep() == earlierTrace.getTimeStep()) continue;
        uint32_t timestep = trace.getTimeStep();
        uint32_t totalTraceRequests = trace.getNumberOfRequests() + trace.getDroppedRequests() + trace.getNumberOfOngoingSimulations() + trace.getEarlyRequests();
        for (auto iter = assignements.rbegin(); iter < assignements.rend(); iter++) {
            if (timestep >= iter->first) {
                assignements.erase(iter.base() - 1);
            }
        }
        if (trace.getNumberOfRequests() == 0) {
            Assignments traceAssignments = trace.getAssignments();

            for (auto traceAssignment : traceAssignments) {
                assignements.push_back(
                    std::make_pair(traceAssignment.getRequestDuration() + timestep, traceAssignment)
                );
            }

            REQUIRE(assignements.size() == trace.getNumberOfOngoingSimulations());
            REQUIRE(parkingAmount == trace.getAvailableParkingSpots() + trace.getNumberOfOngoingSimulations());
        } else {
            REQUIRE(totalTraceRequests >= tempDropAmount);
        }
        tempDropAmount = trace.getDroppedRequests() - tempDropAmount;
        earlierTrace = trace;
    }
}