#ifndef REQUEST_GENERATOR_HPP
#define REQUEST_GENERATOR_HPP

#include <cstdint>
#include <random>
#include <vector>
#include <array>

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
          maxRequestDuration(maxRequestDuration),
          rng(seed),
          requestRate(requestRate) {
            std::vector<double> durationWeights = getDurationBuckets(maxRequestDuration);
            durationDist = std::discrete_distribution<uint64_t>(durationWeights.begin(), durationWeights.end());
        }

    Requests generate(uint64_t currentTimeOfDay);

   private:
    /**
     * Normally poisson is in interval [0, ∞]. When rate > 100 then it becomes a decent
     * approximation of the central limit theorem for gaussian distirbution so we limit it to r
     * ate + 3σ. When rate <= 100 we act like its 100 and limit it to 100 + 3σ
     */
    uint64_t getPoissonUpperBound(double rate) const;

    /**
     * Function that returns a multiplier which changes during the day to represent parking requests
     * as a function of time
     */
    double getTimeMultiplier(uint64_t currentTimeOfDay) const;
    
    /**
     * Uniformly sample duration from a random weighted bucket
     */
    uint64_t getDuration();

    /**
     * Get viable duration buckets
     */
    std::vector<double> getDurationBuckets(uint64_t maxDuration) const;

    std::uniform_int_distribution<uint64_t> dropoffDist;
    std::discrete_distribution<uint64_t> durationDist;
    std::minstd_rand rng;
    
    /**
     * Bucket intervals based on COWI
     */
    const std::array<std::array<uint64_t, 2>, 7> durationBuckets{{
        {{0, 60}},   
        {{61, 120}},  
        {{121, 240}},  
        {{241, 480}}, 
        {{481, 1440}},
        {{1441, 2880}},
        {{2881, std::numeric_limits<uint64_t>::max()}}
    }};

    double requestRate;
    uint64_t maxRequestDuration;
};
}  // namespace palloc

#endif