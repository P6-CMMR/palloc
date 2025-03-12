#include <chrono>
#include <optional>
#include <print>
#include <stdexcept>
#include <string>

#include "argz/argz.hpp"
#include "environment.hpp"
#include "request_generator.hpp"
#include "simulator.hpp"

using namespace palloc;

int main(int argc, char **argv) {
    try {
        argz::about about{"Palloc", "0.0.1"};

        std::optional<std::string> environmentPathOpt;
        std::optional<std::string> outputPathOpt;
        uint64_t timesteps = 1000;
        uint64_t maxDuration = 60;
        uint64_t maxRequestsPerStep = 10;
        uint64_t batchInterval = 2;
        std::optional<uint64_t> seedOpt;
        bool prettify = false;
        bool log = false;
        argz::options opts{
            {{"environment", 'e'}, environmentPathOpt, "the environment file to simulate"},
            {{"timesteps", 't'}, timesteps, "timesteps in minutes to run simulation"},
            {{"duration", 'd'}, maxDuration, "max duration in minutes of requests"},
            {{"requests", 'r'}, maxRequestsPerStep, "max requests to generate per timestep"},
            {{"batch-delay", 'b'}, batchInterval, "interval in minutes before processing requests"},
            {{"seed", 's'}, seedOpt, "seed for randomization, default: unix timestamp"},
            {{"output", 'o'}, outputPathOpt, "the output file to store results in"},
            {{"prettify", 'p'}, prettify, "whether to prettify output or not"},
            {{"log", 'l'}, log, "log detailed execution to stdout"}};

        argz::parse(about, opts, argc, argv);
        if (!environmentPathOpt.has_value() && !about.printed_help) {
            std::println(stderr, "Error: Expected environment file");
            return EXIT_FAILURE;
        }

        if (!environmentPathOpt.has_value()) {
            return EXIT_SUCCESS;
        }

        if (timesteps < 1) {
            std::println(stderr, "Error: Timesteps must be a natural number");
            return EXIT_FAILURE;
        }

        if (maxDuration < 1) {
            std::println(stderr, "Error: Max duration must be a natural number");
            return EXIT_FAILURE;
        }

        Environment env(environmentPathOpt.value());

        const auto seed =
            seedOpt.value_or(std::chrono::system_clock::now().time_since_epoch().count());

        Simulator::simulate(env, {timesteps, maxDuration, maxRequestsPerStep, batchInterval, seed},
                            {outputPathOpt.value_or(""), prettify, log});
    } catch (std::exception &e) {
        std::println(stderr, "Error: {}", e.what());
        return EXIT_FAILURE;
    }
}
