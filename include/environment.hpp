#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include <filesystem>

namespace palloc {
class Environment {
   public:
    explicit Environment(const std::filesystem::path &environmentPath) {}
};
}  // namespace palloc

#endif