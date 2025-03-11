#include "result.hpp"

using namespace palloc;

void Result::saveToFile(const std::filesystem::path &outputPath, bool prettify) const {
    // write_file_json evaluated at compile time so we need both cases
    glz::error_ctx error;
    if (prettify) {
        error = glz::write_file_json<glz::opts{.prettify = true}>(*this, outputPath.string(),
                                                                  std::string{});
    } else {
        error = glz::write_file_json<glz::opts{.prettify = false}>(*this, outputPath.string(),
                                                                   std::string{});
    }

    if (error) {
        const auto errorStr = glz::format_error(error, std::string{});
        throw std::runtime_error("Failed to write to: " + outputPath.string() +
                                 "\nwith error: " + errorStr);
    }
}