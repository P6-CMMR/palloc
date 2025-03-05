#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include "environment.hpp"
#include "request_generator.hpp"
#include "simulator.hpp"

namespace palloc {
class Scheduler {
   public:
    static Simulations scheduleBatch(Environment &env, const Requests &requests);

   private:
    constexpr static int MAX_SEARCH_TIME = 60000;
    constexpr static int PARKING_NODES_TO_VISIT = 1;
};
}  // namespace palloc

#endif