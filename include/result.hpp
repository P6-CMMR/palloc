#ifndef RESULT_HPP
#define RESULT_HPP

#include "trace.hpp"

namespace palloc {
class Result {
    public:
        explicit Result(Traces traces, double globalAvgDuration, double globalAvgCost, size_t droppedRequests) : traces(traces), droppedRequests(droppedRequests), globalAvgDuration(globalAvgDuration), globalAvgCost(globalAvgCost) {}

        void saveToFile(const std::filesystem::path &outputPath) const;
    
    private:
        friend struct glz::meta<Result>;

        Traces traces;
        size_t droppedRequests;
        double globalAvgDuration;
        double globalAvgCost;
};
}

template <>
struct glz::meta<palloc::Result> {
    using T = palloc::Result;
    static constexpr auto value =
        glz::object("traces", &T::traces,
                    "droppedRequests", &T::droppedRequests,
                    "globalAvgDuration", &T::globalAvgDuration,
                    "globalAvgCost", &T::globalAvgCost);
};

#endif