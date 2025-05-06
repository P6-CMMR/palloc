#ifndef REQUEST_GENERATOR_HPP
#define REQUEST_GENERATOR_HPP

#include <array>
#include <memory>
#include <random>
#include <string>

#include "random.hpp"
#include "request.hpp"
#include "types.hpp"

namespace palloc {
struct RequestGeneratorOptions {
    std::string randomGenerator;
    size_t dropoffNodes;
    Uint maxTimeTillArrival;
    Uint maxRequestDuration;
    Uint seed;
    double requestRate;
};

class RequestGenerator {
   public:
    explicit RequestGenerator(const RequestGeneratorOptions &options)
        : _dropoffDist(0, options.dropoffNodes - 1),
          _arrivalDist(0, options.maxTimeTillArrival),
          _maxRequestDuration(options.maxRequestDuration),
          _requestRate(options.requestRate) {
        _rng = random::RandomEngineFactory::create(options.randomGenerator, options.seed);

        DoubleVector durationWeights = getDurationBuckets(options.maxRequestDuration);
        _durationDist =
            std::discrete_distribution<Uint>(durationWeights.begin(), durationWeights.end());
    }

    /**
     * Generate requests based on the current time of day
     *
     * @param currentTimeOfDay time of day in minutes from midnight
     */
    Requests generate(Uint currentTimeOfDay);

    /**
     * Get the number of requests generated
     */
    Uint getRequestsGenerated() const noexcept;

   private:
    /**
     * Sample count from poisson distribution with the rate member variable
     * multiplied by time of day multiplier
     *
     * @param currentTimeOfDay time of day in minutes from midnight
     */
    Uint getCount(Uint currentTimeOfDay);

    /**
     * Uniformly sample one of the dropoffs
     * (sample space is warped because they are not uniformly distributed on a map)
     */
    size_t getDropoff();

    /**
     * Uniformly sample duration from a random weighted bucket
     */
    Uint getDuration();

    /**
     * Uniformly sample the time till arrival
     */
    Uint getArrival();

    /**
     * Function that returns a multiplier which changes during the day to represent parking requests
     * as a function of time
     */
    static double getTimeMultiplier(Uint currentTimeOfDay);

    /**
     * Get viable duration buckets
     */
    static DoubleVector getDurationBuckets(Uint maxDuration);

    std::uniform_int_distribution<size_t> _dropoffDist;
    std::discrete_distribution<Uint> _durationDist;
    std::uniform_int_distribution<Uint> _arrivalDist;
    std::unique_ptr<random::RandomEngine> _rng;

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
    static constexpr std::array<std::array<Uint, 2>, 7> DURATION_BUCKETS{
        {{{0, 60}},                                     // 14%
         {{61, 120}},                                   // 13%
         {{121, 240}},                                  // 11%
         {{241, 480}},                                  // 17%
         {{481, 1440}},                                 // 28%
         {{1441, 2880}},                                // 9%
         {{2881, std::numeric_limits<Uint>::max()}}}};  // 8%

    // Weights based on COWI
    static constexpr std::array<double, 7> originalWeights{0.14, 0.13, 0.11, 0.17,
                                                           0.28, 0.09, 0.08};

    Uint _maxRequestDuration;
    double _requestRate;
    Uint _requestsGenerated = 0;
};
}  // namespace palloc

#endif