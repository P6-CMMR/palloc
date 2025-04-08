#ifndef REQUEST_GENERATOR_HPP
#define REQUEST_GENERATOR_HPP

#include <array>
#include <cstdint>
#include <random>
#include <vector>

#include "types.hpp"

namespace palloc {

class Request {
   public:
    explicit Request(uint32_t dropoffNode, uint32_t requestDuration, uint32_t tillArrival)
        : _dropoffNode(dropoffNode), _requestDuration(requestDuration), _tillArrival(tillArrival) {}

    uint32_t getDropoffNode() const noexcept;
    uint32_t getRequestDuration() const noexcept;
    uint32_t getTimesDropped() const noexcept;
    uint32_t getArrival() const noexcept;

    void decrementDuration() noexcept;
    void decrementTillArrival() noexcept;
    void incrementTimesDropped() noexcept;

    bool isDead() const noexcept;
    bool isEarly() const noexcept;

   private:
    uint32_t _dropoffNode;
    uint32_t _requestDuration;
    uint32_t _timesDropped = 0;
    uint32_t _tillArrival;
};

using Requests = std::vector<Request>;

class RequestGenerator {
   public:
    struct Options {
        size_t dropoffNodes;
        uint32_t maxTimeTillArrival;
        uint32_t maxRequestDuration;
        uint32_t seed;
        double requestRate;
    };

    explicit RequestGenerator(const RequestGenerator::Options &options)
        : _dropoffDist(0, options.dropoffNodes - 1),
          _arrivalDist(0, options.maxTimeTillArrival),
          _rng(options.seed),
          _maxRequestDuration(options.maxRequestDuration),
          _requestRate(options.requestRate) {
        DoubleVector durationWeights = getDurationBuckets(options.maxRequestDuration);
        _durationDist =
            std::discrete_distribution<uint32_t>(durationWeights.begin(), durationWeights.end());
    }

    Requests generate(uint32_t currentTimeOfDay);

   private:
    /**
     * Sample count from poisson distribution with the rate member variable
     * multiplied by time of day multiplier
     *
     * @param currentTimeOfDay time of day in minutes from midnight
     */
    uint32_t getCount(uint32_t currentTimeOfDay);

    /**
     * Uniformly sample one of the dropoffs
     * (sample space is warped because they are not uniformly distributed on a map)
     */
    size_t getDropoff();

    /**
     * Uniformly sample duration from a random weighted bucket
     */
    uint32_t getDuration();

    /**
     * Uniformly sample the time till arrival
     */
    uint32_t getArrival();

    /**
     * Normally poisson is in interval [0, ∞]. When rate > 100 then it becomes a decent
     * approximation of the central limit theorem for gaussian distirbution so we limit it to r
     * ate + 3σ. When rate <= 100 we act like its 100 and limit it to 100 + 3σ
     */
    static uint32_t getPoissonUpperBound(double rate);

    /**
     * Function that returns a multiplier which changes during the day to represent parking requests
     * as a function of time
     */
    static double getTimeMultiplier(uint32_t currentTimeOfDay);

    /**
     * Get viable duration buckets
     */
    static DoubleVector getDurationBuckets(uint32_t maxDuration);

    std::uniform_int_distribution<size_t> _dropoffDist;
    std::discrete_distribution<uint32_t> _durationDist;
    std::uniform_int_distribution<uint32_t> _arrivalDist;
    std::minstd_rand _rng;

    // Bucket intervals based on COWI
    static constexpr std::array<std::array<uint32_t, 2>, 7> DURATION_BUCKETS{
        {{{0, 60}},                                         // 14%
         {{61, 120}},                                       // 13%
         {{121, 240}},                                      // 10%
         {{241, 480}},                                      // 16%
         {{481, 1440}},                                     // 21%
         {{1441, 2880}},                                    // 9%
         {{2881, std::numeric_limits<uint32_t>::max()}}}};  // 7%

    // Weights based on COWI
    static constexpr std::array<double, 7> originalWeights{14.0, 13.0, 10.0, 16.0, 21.0, 9.0, 7.0};

    uint32_t _maxRequestDuration;
    double _requestRate;
};
}  // namespace palloc

#endif