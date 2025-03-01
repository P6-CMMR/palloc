#include "palloc.hpp"

using namespace palloc;

int main(int argc, char **argv) {
    argz::about about{"Palloc", "0.0.1"};

    std::optional<std::string> environmentPathOpt;
    uint64_t timesteps = 1000;
    std::optional<uint64_t> seed;
    argz::options opts{
        {{"environment", 'e'}, environmentPathOpt, "the environment file to simulate"},
        {{"timesteps", 't'}, timesteps, "timesteps to simulate"},
        {{"seed", 's'}, seed, "seed for randomization, default: unix timestamp"}};

    try {
        argz::parse(about, opts, argc, argv);
        if (!environmentPathOpt.has_value() && !about.printed_help) {
            std::println("Error: Expected environment file");
            return EXIT_FAILURE;
        }

        if (!environmentPathOpt.has_value()) { return EXIT_SUCCESS; }

        Environment env(environmentPathOpt.value());
        Simulator::simulate(env, timesteps);
    } catch (std::exception &e) {
        std::println(stderr, "Error: {}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
