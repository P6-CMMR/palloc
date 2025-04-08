#include "catch2/catch_test_macros.hpp"
#include "simulator.hpp"

using namespace palloc;

TEST_CASE("Base case - [Simulator]") {
    const std::filesystem::path test_data_path = std::filesystem::path(PROJECT_ROOT) / "tests/test_data.json";
    const std::filesystem::path temp_result_path = std::filesystem::path(PROJECT_ROOT) / "tests/temp_result.json";

    OutputSettings outputSettings{.path = temp_result_path, .numberOfRunsToAggregate = 1, .prettify = false};
    GeneralSettings generalSettings{.numberOfThreads = 1};

    SECTION("Base case - [Simulator]") {
        SimulatorSettings simSettings{
            .timesteps = 5,
            .maxRequestDuration = 5,
            .requestRate = 10,
            .maxTimeTillArrival = 5,
            .batchInterval = 2,
            .seed = 1
        };
    
        Environment env(test_data_path);
    
        Simulator::simulate(env, simSettings, outputSettings, generalSettings);

        Result result(temp_result_path);

        const auto traces = result.getTraceLists();

    }

}