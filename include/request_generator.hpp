#ifndef REQUEST_GENERATOR_HPP
#define REQUEST_GENERATOR_HPP

#include <cstdint>
#include <random>
#include <vector>

namespace palloc {

class Request {
   public:
    explicit Request(uint64_t dropoffNode, uint64_t requestDuration)
        : dropoffNode(dropoffNode), requestDuration(requestDuration) {}

    uint64_t getDropoffNode() const noexcept;
    uint64_t getRequestDuration() const noexcept;
    uint64_t getTimesDropped() const noexcept;

    void decrementDuration() noexcept;
    void incrementTimesDropped() noexcept;

    bool isDead() const noexcept;

   private:
    uint64_t dropoffNode;
    uint64_t requestDuration;
    uint64_t timesDropped = 0;
};

using Requests = std::vector<Request>;

class RequestGenerator {
   public:
    explicit RequestGenerator(uint64_t dropoffNodes, uint64_t maxRequestDuration,
                              uint64_t maxRequestsPerStep, uint64_t seed)
        : dropoffDist(0, dropoffNodes - 1),
          durationDist(1, maxRequestDuration),
          requestCountDist(0, maxRequestsPerStep),
          rng(seed) {}

    Requests generate(uint64_t currentTimeOfDay);

    static double getTimeMultiplier(uint64_t currentTimeOfDay);

   private:
    std::uniform_int_distribution<uint64_t> dropoffDist;
    std::uniform_int_distribution<uint64_t> durationDist;
    std::uniform_int_distribution<uint64_t> requestCountDist;
    std::minstd_rand rng;
};
}  // namespace palloc

#endif