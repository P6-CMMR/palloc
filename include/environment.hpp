#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include <string_view>

namespace palloc {
class Environment {
   public:
    explicit Environment(std::string_view environmentPath) {}
};
}  // namespace palloc

#endif