#ifndef REQUEST_GENERATOR_HPP
#define REQUEST_GENERATOR_HPP

#include <array>
#include <cstdint>
#include <random>

#include "request.hpp"
#include "types.hpp"

namespace palloc {
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

    // Traffic weights for Aalborg based on tomtom
    static constexpr std::array<double, 24> TRAFFIC_WEIGHTS{
        78.0 / 365.0,   // 00:00
        66.0 / 365.0,   // 01:00
        51.0 / 365.0,   // 02:00
        69.0 / 365.0,   // 03:00
        39.0 / 365.0,   // 04:00
        78.0 / 365.0,   // 05:00
        246.0 / 365.0,  // 06:00
        708.0 / 365.0,  // 07:00
        558.0 / 365.0,  // 08:00
        432.0 / 365.0,  // 09:00
        501.0 / 365.0,  // 10:00
        108.0 / 73.0,   // 11:00
        582.0 / 365.0,  // 12:00
        117.0 / 73.0,   // 13:00
        138.0 / 73.0,   // 14:00
        183.0 / 73.0,   // 15:00
        141.0 / 73.0,   // 16:00
        501.0 / 365.0,  // 17:00
        381.0 / 365.0,  // 18:00
        297.0 / 365.0,  // 19:00
        264.0 / 365.0,  // 20:00
        213.0 / 365.0,  // 21:00
        156.0 / 365.0,  // 22:00
        21.0 / 73.0     // 23:00
    };

    // Bucket intervals based on COWI
    static constexpr std::array<std::array<uint32_t, 2>, 7> DURATION_BUCKETS{
        {{{0, 60}},                                         // 14%
         {{61, 120}},                                       // 13%
         {{121, 240}},                                      // 11%
         {{241, 480}},                                      // 17%
         {{481, 1440}},                                     // 28%
         {{1441, 2880}},                                    // 9%
         {{2881, std::numeric_limits<uint32_t>::max()}}}};  // 8%

    // Weights based on COWI
    static constexpr std::array<double, 7> originalWeights{0.14, 0.13, 0.11, 0.17,
                                                           0.28, 0.09, 0.08};

    uint32_t _maxRequestDuration;
    double _requestRate;
};
}  // namespace palloc

#endif