#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <cstdint>

#include "environment.hpp"

namespace palloc {
class Simulator {
   public:
    static void simulate(const Environment &env, uint64_t timesteps);
};
}  // namespace palloc

#endif