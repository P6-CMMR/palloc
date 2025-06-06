#include "palloc.hpp"

using namespace palloc;

int main(int argc, char **argv) {
    try {
        argz::about about{"Palloc", "0.0.1"};

        std::string environmentPathStr;
        std::string outputPathStr;
        SimulatorSettings simSettings{.timesteps = 1440,
                                      .maxRequestDuration = 2880,
                                      .requestRate = 4.0,
                                      .maxTimeTillArrival = 0,
                                      .minParkingTime = 0,
                                      .batchInterval = 2,
                                      .commitInterval = 0,
                                      .useWeightedParking = false,
                                      .randomGenerator = "pcg"};

        OutputSettings outputSettings{
            .numberOfRunsToAggregate = 3, .prettify = false, .outputTrace = false};

        std::optional<Uint> seedOpt;
        std::string startTimeStr = "08:00";

        std::optional<Uint> numberOfThreadsOpt;

        argz::options opts{
            {{"environment", 'e'}, environmentPathStr, "the environment file to simulate"},
            {{"timesteps", 't'}, simSettings.timesteps, "timesteps in minutes to run simulation"},
            {{"start-time", 'S'}, startTimeStr, "time to start simulation"},
            {{"duration", 'd'},
             simSettings.maxRequestDuration,
             "max duration in minutes of requests"},
            {{"arrival", 'A'},
             simSettings.maxTimeTillArrival,
             "max duration of early requests in minutes"},
            {{"minimum-parking-time", 'm'},
             simSettings.minParkingTime,
             "minimum parking time in minutes"},
            {{"request-rate", 'r'},
             simSettings.requestRate,
             "rate of requests to generate per timestep"},
            {{"batch-interval", 'b'},
             simSettings.batchInterval,
             "interval in minutes before processing requests"},
            {{"commit-interval", 'c'},
             simSettings.commitInterval,
             "interval before arriving a request can be committed to a parking spot"},
            {{"weighted-parking", 'w'},
             simSettings.useWeightedParking,
             "use weighted parking cost depending on dropoff node density"},
            {{"random-generator", 'g'},
             simSettings.randomGenerator,
             "random generator to use (options: pcg, pcg-fast)"},
            {{"seed", 's'}, seedOpt, "seed for randomization, default: unix timestamp"},
            {{"output", 'o'},
             outputPathStr,
             "the output file to store results in, default: no output"},
            {{"trace", 'T'}, outputSettings.outputTrace, "whether to output trace or not"},
            {{"prettify", 'p'}, outputSettings.prettify, "whether to prettify output or not"},
            {{"aggregate", 'a'},
             outputSettings.numberOfRunsToAggregate,
             "number of runs to aggregate together"},
            {{"jobs", 'j'},
             numberOfThreadsOpt,
             "number of threads to use for aggregation, default: min(number of hardware threads, "
             "number of aggregates)"}};

        argz::parse(about, opts, argc, argv);
        if (about.printed_help || about.printed_version) {
            return EXIT_SUCCESS;
        }

        if (environmentPathStr.empty()) {
            std::println(stderr, "Error: Expected environment file");
            return EXIT_FAILURE;
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

        if (!random::availableGenerators.contains(simSettings.randomGenerator)) {
            std::println(stderr, "Error: Random generator must be either pcg or pcg-fast");
            return EXIT_FAILURE;
        }

        if (outputSettings.numberOfRunsToAggregate < 1) {
            std::println(stderr, "Error: Number of aggregates must be a natural number");
            return EXIT_FAILURE;
        }

        Environment env(environmentPathStr);

        simSettings.seed =
            seedOpt.value_or(std::chrono::system_clock::now().time_since_epoch().count());
        outputSettings.outputPath = outputPathStr;

        GeneralSettings generalSettings{
            .numberOfThreads = numberOfThreadsOpt.value_or(std::min(
                std::thread::hardware_concurrency(), outputSettings.numberOfRunsToAggregate))};

        Simulator::simulate(env, simSettings, outputSettings, generalSettings);
    } catch (std::exception &e) {
        std::println(stderr, "Error: {}", e.what());
        return EXIT_FAILURE;
    } catch (...) {
        std::println(stderr, "Error: Unknown exception occurred");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
