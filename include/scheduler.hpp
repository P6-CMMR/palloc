#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include "environment.hpp"
#include "request_generator.hpp"
#include "simulator.hpp"

namespace palloc {
struct SchedulerResult {
    Simulations simulations;
    Requests unassignedRequests;
    Requests earlyRequests;
    double averageDurations;
    double cost;
};

class Scheduler {
   public:
    static SchedulerResult scheduleBatch(Environment &env, Requests &requests);

   private:
    static constexpr int MAX_SEARCH_TIME = 60000;
    static constexpr int PARKING_NODES_TO_VISIT = 1;
    static constexpr int UNASSIGNED_PENALTY = 100;
};
}  // namespace palloc

#endif