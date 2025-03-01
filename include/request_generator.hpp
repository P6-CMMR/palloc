#ifndef REQUEST_GENERATOR_HPP
#define REQUEST_GENERATOR_HPP

#include <cstdint>
#include <generator>
#include <random>
#include <utility>

namespace palloc {
class RequestGenerator {
   public:
    using Request = std::pair<size_t, size_t>;
    using Seed = uint64_t;

    explicit RequestGenerator(size_t dropoffNodes, size_t maxDuration, size_t maxRequestsPerStep,
                              Seed seed)
        : dropoffDist(0, dropoffNodes - 1),
          durationDist(1, maxDuration - 1),
          requestCountDist(0, maxRequestsPerStep - 1),
          rng(seed) {}

    std::generator<Request> generate();

   private:
    std::uniform_int_distribution<size_t> dropoffDist;
    std::uniform_int_distribution<size_t> durationDist;
    std::uniform_int_distribution<size_t> requestCountDist;
    std::minstd_rand rng;
};
}  // namespace palloc

#endif