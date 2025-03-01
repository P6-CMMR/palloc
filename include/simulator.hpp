#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

<<<<<<< Updated upstream
#include <cstdint>

=======
>>>>>>> Stashed changes
#include "environment.hpp"

namespace palloc {
class Simulator {
   public:
    static void simulate(const Environment &env, size_t timesteps, size_t maxDuration,
                         size_t maxRequestsPerStep, size_t seed);
};
}  // namespace palloc

#endif