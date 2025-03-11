#ifndef RESULT_HPP
#define RESULT_HPP

#include <cstdint>

#include "trace.hpp"

namespace palloc {
class Result {
   public:
    explicit Result(Traces traces, uint64_t seed, size_t droppedRequests, double globalAvgDuration,
                    double globalAvgCost)
        : traces(std::move(traces)),
          seed(seed),
          droppedRequests(droppedRequests),
          globalAvgDuration(globalAvgDuration),
          globalAvgCost(globalAvgCost) {}

    void saveToFile(const std::filesystem::path &outputPath, bool prettify) const;

   private:
    friend struct glz::meta<Result>;

    Traces traces;
    uint64_t seed;
    size_t droppedRequests;
    double globalAvgDuration;
    double globalAvgCost;
};
}  // namespace palloc

template <>
struct glz::meta<palloc::Result> {
    using T = palloc::Result;
    static constexpr auto value = glz::object(
        "seed", &T::seed, "droppedRequests", &T::droppedRequests, "globalAvgDuration",
        &T::globalAvgDuration, "globalAvgCost", &T::globalAvgCost, "traces", &T::traces);
};

#endif