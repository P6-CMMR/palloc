#include <argz/argz.hpp>
#include <chrono>
#include <optional>
#include <print>
#include <stdexcept>
#include <string>

#include "environment.hpp"
#include "request_generator.hpp"
#include "simulator.hpp"

using namespace palloc;

int main(int argc, char **argv) {
    argz::about about{"Palloc", "0.0.1"};

    std::optional<std::string> environmentPathOpt;
    uint64_t timesteps = 1000;
    uint64_t maxDuration = 60;
    uint64_t maxRequestsPerStep = 10;
    uint64_t batchDelay = 180;
    std::optional<uint64_t> seedOpt;
    argz::options opts{
        {{"environment", 'e'}, environmentPathOpt, "the environment file to simulate"},
        {{"timesteps", 't'}, timesteps, "timesteps to simulate"},
        {{"duration", 'd'}, maxDuration, "max duration of requests"},
        {{"requests", 'r'}, maxRequestsPerStep, "max requests to generate per timestep"},
        {{"batch-delay", 'b'}, batchDelay, "delay before processing requests"},
        {{"seed", 's'}, seedOpt, "seed for randomization, default: unix timestamp"}};

    try {
        argz::parse(about, opts, argc, argv);
        if (!environmentPathOpt.has_value() && !about.printed_help) {
            std::println("Error: Expected environment file");
            return EXIT_FAILURE;
        }

        if (!environmentPathOpt.has_value()) {
            return EXIT_SUCCESS;
        }

        Environment env(environmentPathOpt.value());

        const auto seed =
            seedOpt.value_or(std::chrono::system_clock::now().time_since_epoch().count());

        Simulator::simulate(env, timesteps, maxDuration, maxRequestsPerStep, batchDelay, seed);
    } catch (std::exception &e) {
        std::println(stderr, "Error: {}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
