#ifndef REQUEST_GENERATOR_HPP
#define REQUEST_GENERATOR_HPP

#include <cstdint>
#include <random>
#include <utility>
#include <vector>

namespace palloc {
class RequestGenerator {
   public:
    using Request = std::pair<size_t, uint64_t>;
    using Requests = std::vector<Request>;
    using Seed = uint64_t;

    explicit RequestGenerator(size_t dropoffNodes, uint64_t maxDuration, uint64_t maxRequestsPerStep,
                              Seed seed)
        : dropoffDist(0, dropoffNodes - 1),
          durationDist(1, maxDuration),
          requestCountDist(0, maxRequestsPerStep),
          rng(seed) {}

    const Requests generate();

   private:
    std::uniform_int_distribution<uint64_t> dropoffDist;
    std::uniform_int_distribution<uint64_t> durationDist;
    std::uniform_int_distribution<uint64_t> requestCountDist;
    std::minstd_rand rng;
};
}  // namespace palloc

#endif