#include "palloc.hpp"

using namespace palloc;

int main(int argc, char **argv) {
    constexpr std::string_view version = "0.0.1";
    argz::about about{"Palloc", version};

    std::optional<std::string> environmentPathOpt;
    uint64_t timesteps = 1000;
    argz::options opts{
        {{"environment", 'e'}, environmentPathOpt, "the environment file to simulate"},
        {{"timesteps", 't'}, timesteps, "timesteps to simulate"}};

    try {
        argz::parse(about, opts, argc, argv);
    } catch (std::exception &e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    if (!environmentPathOpt.has_value() && !about.printed_help) {
        std::cout << "Usage: -e <environment file>\n";
        return EXIT_FAILURE;
    }

    Environment env(environmentPathOpt.value());
    Simulator::simulate(env, timesteps);

    return EXIT_SUCCESS;
}
