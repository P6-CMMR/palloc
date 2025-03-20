#ifndef REQUEST_GENERATOR_HPP
#define REQUEST_GENERATOR_HPP

#include <cstdint>
#include <random>
#include <vector>

namespace palloc {

class Request {
   public:
    explicit Request(uint64_t dropoffNode, uint64_t duration, uint64_t tillArrival)
        : dropoffNode(dropoffNode), duration(duration), tillArrival(tillArrival) {}

    uint64_t getDropoffNode() const noexcept;
    uint64_t getDuration() const noexcept;
    uint64_t getArrival() const noexcept;
    uint64_t getTimesDropped() const noexcept;

    void decrementDuration() noexcept;
    void decrementTillArrival() noexcept;
    void incrementTimesDropped() noexcept;

    bool isDead() const noexcept;
    bool isReserved() const noexcept;

   private:
    uint64_t dropoffNode;
    uint64_t duration;
    uint64_t timesDropped = 0;
    uint64_t tillArrival;
};

using Requests = std::vector<Request>;

class RequestGenerator {
   public:
    explicit RequestGenerator(uint64_t dropoffNodes, uint64_t maxRequestDuration, uint64_t maxTimeTillArrival,
                              uint64_t maxRequestsPerStep, uint64_t seed)
        : dropoffDist(0, dropoffNodes - 1),
          durationDist(1, maxRequestDuration),
          arrivalDist(0, maxTimeTillArrival),
          requestCountDist(0, maxRequestsPerStep),
          rng(seed) {}

    Requests generate();

   private:
    std::uniform_int_distribution<uint64_t> dropoffDist;
    std::uniform_int_distribution<uint64_t> durationDist;
    std::uniform_int_distribution<uint64_t> arrivalDist;
    std::uniform_int_distribution<uint64_t> requestCountDist;
    std::minstd_rand rng;
};
}  // namespace palloc

#endif