#ifndef REQUEST_GENERATOR_HPP
#define REQUEST_GENERATOR_HPP

#include <array>
#include <cstdint>
#include <random>
#include <vector>

namespace palloc {

class Request {
   public:
    explicit Request(uint64_t dropoffNode, uint64_t requestDuration)
        : _dropoffNode(dropoffNode), _requestDuration(requestDuration) {}

    uint64_t getDropoffNode() const noexcept;
    uint64_t getRequestDuration() const noexcept;
    uint64_t getTimesDropped() const noexcept;

    void decrementDuration() noexcept;
    void incrementTimesDropped() noexcept;

    bool isDead() const noexcept;

   private:
    uint64_t _dropoffNode;
    uint64_t _requestDuration;
    uint64_t _timesDropped = 0;
};

using Requests = std::vector<Request>;

class RequestGenerator {
   public:
    explicit RequestGenerator(uint64_t dropoffNodes, uint64_t maxRequestDuration, uint64_t seed,
                              double requestRate)
        : _dropoffDist(0, dropoffNodes - 1),
          _rng(seed),
          _requestRate(requestRate),
          _maxRequestDuration(maxRequestDuration) {
        std::vector<double> durationWeights = getDurationBuckets(maxRequestDuration);
        _durationDist =
            std::discrete_distribution<uint64_t>(durationWeights.begin(), durationWeights.end());
    }

    Requests generate(uint64_t currentTimeOfDay);

   private:
    /**
     * Normally poisson is in interval [0, ∞]. When rate > 100 then it becomes a decent
     * approximation of the central limit theorem for gaussian distirbution so we limit it to r
     * ate + 3σ. When rate <= 100 we act like its 100 and limit it to 100 + 3σ
     */
    static uint64_t getPoissonUpperBound(double rate);

    /**
     * Function that returns a multiplier which changes during the day to represent parking requests
     * as a function of time
     */
    static double getTimeMultiplier(uint64_t currentTimeOfDay);

    /**
     * Uniformly sample duration from a random weighted bucket
     */
    uint64_t getDuration();

    /**
     * Get viable duration buckets
     */
    std::vector<double> getDurationBuckets(uint64_t maxDuration) const;

    std::uniform_int_distribution<uint64_t> _dropoffDist;
    std::discrete_distribution<uint64_t> _durationDist;
    std::minstd_rand _rng;

    /**
     * Bucket intervals based on COWI
     */
    static constexpr std::array<std::array<uint64_t, 2>, 7> DURATION_BUCKETS{{
        {{0, 60}},
        {{61, 120}},
        {{121, 240}},
        {{241, 480}},
        {{481, 1440}},
        {{1441, 2880}},
        {{2881, std::numeric_limits<uint64_t>::max()}}
    }};

    double _requestRate;
    uint64_t _maxRequestDuration;
};
}  // namespace palloc

#endif