#ifndef REQUEST_GENERATOR_HPP
#define REQUEST_GENERATOR_HPP

#include <cstdint>
#include <random>

namespace palloc {
struct Request {
    size_t dropoffNode;
    uint64_t duration;

    explicit Request(size_t dropoffNode, uint64_t duration)
        : dropoffNode(dropoffNode), duration(duration) {}
};

using Requests = std::vector<Request>;

class RequestGenerator {
   public:
    explicit RequestGenerator(size_t dropoffNodes, uint64_t maxDuration,
                              uint64_t maxRequestsPerStep, uint64_t seed)
        : dropoffDist(0, dropoffNodes - 1),
          durationDist(1, maxDuration),
          requestCountDist(0, maxRequestsPerStep),
          rng(seed) {}

    Requests generate();

   private:
    std::uniform_int_distribution<uint64_t> dropoffDist;
    std::uniform_int_distribution<uint64_t> durationDist;
    std::uniform_int_distribution<uint64_t> requestCountDist;
    std::minstd_rand rng;
};
}  // namespace palloc

#endif