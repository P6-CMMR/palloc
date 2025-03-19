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
    explicit RequestGenerator(uint64_t dropoffNodes, uint64_t maxRequestDuration, uint64_t seed,
                              double requestRate)
        : dropoffDist(0, dropoffNodes - 1),
          durationDist(1, maxRequestDuration),
          rng(seed),
          requestRate(requestRate) {}

    Requests generate(uint64_t currentTimeOfDay);
    
    /**
     * Normally poisson is in interval [0, ∞]. When rate > 100 then it approximates central limit
     * theorem for gaussian distirbution so we limit it to rate + 3σ. When rate <= 100 we act like
     * its 100 and limit it to 100 + 3σ
     */
    static uint64_t getPoissonUpperBound(double rate);

    /**
     * Function that returns a multiplier which changes during the day to represent parking requests
     * as a function of time
     */
    static double getTimeMultiplier(uint64_t currentTimeOfDay);

   private:
    std::uniform_int_distribution<uint64_t> dropoffDist;
    std::uniform_int_distribution<uint64_t> durationDist;
    std::minstd_rand rng;

    double requestRate;
};
}  // namespace palloc

#endif