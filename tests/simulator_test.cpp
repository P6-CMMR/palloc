#include "simulator.hpp"
#include "aggregated_result.hpp"

#include "catch2/catch_test_macros.hpp"

using namespace palloc;

TEST_CASE("Base case - [Simulator]") {
    const Path testDataPath =
        Path(PROJECT_ROOT) / "tests/test_data.json";
    const Path tempResultPath =
        Path(PROJECT_ROOT) / "tests/temp_result.json";

    OutputSettings outputSettings{
        .outputPath = tempResultPath, .numberOfRunsToAggregate = 1, .prettify = false, .outputTrace = true};
    GeneralSettings generalSettings{.numberOfThreads = 1};

    constexpr auto timesteps = 1000;
    constexpr auto startTime = 0;
    constexpr auto maxRequestDuration = 5;
    constexpr auto requestRate = 10;
    constexpr auto maxTimeTillArrival = 5;
    constexpr auto minParkingTime = 0;
    constexpr auto batchInterval = 2;
    constexpr auto seed = 1;
    constexpr auto useWeightedParking = false;
    constexpr auto commitInterval = 0;

    SimulatorSettings simSettings{.timesteps = timesteps,
                                  .startTime = startTime,
                                  .maxRequestDuration = maxRequestDuration,
                                  .requestRate = requestRate,
                                  .maxTimeTillArrival = maxTimeTillArrival,
                                  .minParkingTime = minParkingTime,
                                  .batchInterval = batchInterval,
                                  .commitInterval = commitInterval,
                                  .seed = seed,
                                  .useWeightedParking = useWeightedParking,
                                  .randomGenerator = "pcg"};

    Environment env(testDataPath);

    Simulator::simulate(env, simSettings, outputSettings, generalSettings);

    AggregatedResult result(tempResultPath);

    const auto traces = result.getTraceLists()[0];
    REQUIRE(traces.size() == timesteps);

    Trace earlierTrace = traces.front();
    std::vector<std::pair<Uint, Assignment>> assignements;

    const size_t parkingAmount =
        earlierTrace.getAvailableParkingSpots() + earlierTrace.getNumberOfOngoingSimulations();
    size_t tempDropAmount = 0;

    for (const auto &trace : traces) {
        if (trace.getTimeStep() == earlierTrace.getTimeStep()) {
            continue;
        }
        
        Uint timestep = trace.getTimeStep();
        size_t totalTraceRequests = trace.getNumberOfRequests() + trace.getDroppedRequests() +
                                    trace.getNumberOfOngoingSimulations() +
                                    trace.getEarlyRequests();
        for (auto iter = assignements.rbegin(); iter < assignements.rend(); iter++) {
            if (timestep >= iter->first) {
                assignements.erase(iter.base() - 1);
            }
        }

        if (trace.getNumberOfRequests() == 0) {
            Assignments traceAssignments = trace.getAssignments();
            for (auto &traceAssignment : traceAssignments) {
                assignements.push_back(std::make_pair(
                    traceAssignment.getRequestDuration() + timestep, traceAssignment));
            }

            REQUIRE(assignements.size() == trace.getNumberOfOngoingSimulations());
            REQUIRE(parkingAmount ==
                    trace.getAvailableParkingSpots() + trace.getNumberOfOngoingSimulations());
        } else {
            REQUIRE(totalTraceRequests >= tempDropAmount);
        }

        tempDropAmount = trace.getDroppedRequests() - tempDropAmount;
        earlierTrace = trace;
    }
}