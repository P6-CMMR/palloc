#include "result.hpp"

using namespace palloc;

void Result::saveToFile(const std::filesystem::path &outputPath) const {
    const auto error = glz::write_file_json(*this, outputPath.string(), std::string{});
    if (error) {
        const auto errorStr = glz::format_error(error, std::string{});
        throw std::runtime_error("Failed to write to: " + outputPath.string() +
                                 "\nwith error: " + errorStr);
    }
}