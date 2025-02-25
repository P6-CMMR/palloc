#include "palloc.hpp"

int main(int argc, char **argv) {
    constexpr std::string_view version = "0.0.1";
    argz::about about{"Palloc", version};

    std::optional<std::string> environmentPath;
    argz::options opts{{{"environment", 'e'}, environmentPath, "the environment file to simulate"}};

    try {
        argz::parse(about, opts, argc, argv);
    } catch (std::exception &e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    if (!environmentPath.has_value()) { 
        return EXIT_SUCCESS;
    }
}
