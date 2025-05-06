#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_set>

#include "types.hpp"

namespace palloc::random {

/**
 * Available random generators
 */
static std::unordered_set<std::string> availableGenerators = {"pcg", "pcg-fast"};

/**
 * Random engine interface
 */
class RandomEngine {
   public:
    // needs to be defined for stl
    using result_type = Uint;

    virtual ~RandomEngine() = default;

    static constexpr Uint min() { return 0; }
    static constexpr Uint max() { return std::numeric_limits<Uint>::max(); }

    virtual Uint operator()() = 0;
};

class RandomEngineFactory {
   public:
    static std::unique_ptr<RandomEngine> create(std::string_view generatorName, Uint seed);
};

/**
 * Permuted Congruential Generator
 *
 * Implementation based on: https://en.wikipedia.org/wiki/Permuted_congruential_generator
 */
class PcgEngine : public RandomEngine {
   public:
    explicit PcgEngine(Uint seed);

    Uint operator()() override;

   private:
    static Uint rotr32(Uint x, Uint r) noexcept;

    static constexpr Uint64 _multiplier = 6364136223846793005U;
    static constexpr Uint64 _increment = 1442695040888963407U;

    Uint64 _state;
};

/**
 * Fast Permuted Congruential Generator
 *
 * Implementation based on: https://en.wikipedia.org/wiki/Permuted_congruential_generator
 */
class PcgEngineFast : public RandomEngine {
   public:
    explicit PcgEngineFast(Uint seed);

    Uint operator()() override;

   private:
    static constexpr Uint64 _multiplier = 6364136223846793005U;

    Uint64 _state;
};

}  // namespace palloc::random

#endif