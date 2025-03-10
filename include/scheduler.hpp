#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include "environment.hpp"
#include "request_generator.hpp"
#include "simulator.hpp"

namespace palloc {
struct SchedulerResult {
    Simulations simulations;
    Requests unassignedRequests;
    double averageDurations;
    size_t cost;
};

class Scheduler {
   public:
    static SchedulerResult scheduleBatch(Environment &env, const Requests &requests);

   private:
    constexpr static int MAX_SEARCH_TIME = 60000;
    constexpr static int PARKING_NODES_TO_VISIT = 1;
    constexpr static double UNASSIGNED_PENALTY = 100000.0;
};
}  // namespace palloc

#endif