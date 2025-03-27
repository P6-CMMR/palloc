#include "palloc.hpp"

using namespace palloc;

int main(int argc, char **argv) {
    try {
        argz::about about{"Palloc", "0.0.1"};

        std::string environmentPathStr;
        std::string outputPathStr;
        SimulatorSettings simSettings{
            .timesteps = 1440, .maxRequestDuration = 600, .requestRate = 10, .batchInterval = 2};

        OutputSettings outputSettings{.numberOfRunsToAggregate = 1, .prettify = false};

        std::optional<uint64_t> seedOpt;
        std::string startTimeStr = "08:00";

        argz::options opts{
            {{"environment", 'e'}, environmentPathStr, "the environment file to simulate"},
            {{"timesteps", 't'}, simSettings.timesteps, "timesteps in minutes to run simulation"},
            {{"start-time", 'S'}, startTimeStr, "time to start simulation"},
            {{"duration", 'd'},
             simSettings.maxRequestDuration,
             "max duration in minutes of requests"},
            {{"requests", 'r'},
             simSettings.requestRate,
             "rate of requests to generate per timestep"},
            {{"batch-delay", 'b'},
             simSettings.batchInterval,
             "interval in minutes before processing requests"},
            {{"seed", 's'}, seedOpt, "seed for randomization, default: unix timestamp"},
            {{"output", 'o'},
             outputPathStr,
             "the output file to store results in, default: no output"},
            {{"prettify", 'p'}, outputSettings.prettify, "whether to prettify output or not"},
            {{"aggregate", 'a'},
             outputSettings.numberOfRunsToAggregate,
             "number of runs to aggregate together"}};

        argz::parse(about, opts, argc, argv);
        if (environmentPathStr.empty() && !about.printed_help) {
            std::println(stderr, "Error: Expected environment file");
            return EXIT_FAILURE;
        }

        if (environmentPathStr.empty()) {
            return EXIT_SUCCESS;
        }

        if (simSettings.timesteps < 1) {
            std::println(stderr, "Error: Timesteps must be a natural number");
            return EXIT_FAILURE;
        }

        if (simSettings.maxRequestDuration < 1) {
            std::println(stderr, "Error: Max duration must be a natural number");
            return EXIT_FAILURE;
        }

        simSettings.startTime = DateParser::parseTimeToMinutes(startTimeStr);
        if (simSettings.startTime > 1439) {
            std::println(stderr,
                         "Error: Start time must be a positive integer in the range [1..1439]");
            return EXIT_FAILURE;
        }

        if (simSettings.requestRate <= 0) {
            std::println(stderr, "Error: Request rate must be a positive real");
            return EXIT_FAILURE;
        }

        if (outputSettings.numberOfRunsToAggregate < 1) {
            std::println(stderr, "Error: Number of aggregates must be a natural number");
            return EXIT_FAILURE;
        }

        Environment env(environmentPathStr);

        simSettings.seed =
            seedOpt.value_or(std::chrono::system_clock::now().time_since_epoch().count());
        outputSettings.path = outputPathStr;

        Simulator::simulate(env, simSettings, outputSettings);
    } catch (std::exception &e) {
        std::println(stderr, "Error: {}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
