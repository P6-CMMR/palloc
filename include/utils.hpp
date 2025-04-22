#ifndef UTILS_HPP
#define UTILS_HPP

#include <concepts>
#include <ranges>

namespace palloc {
// Generic implementation of: https://en.wikipedia.org/wiki/Kahan_summation_algorithm

class utils {
   public:
    template <class Range>
        requires std::ranges::range<Range> && std::floating_point<std::ranges::range_value_t<Range>>
    static auto kanhanSum(Range &values) {
        using Fp = std::ranges::range_value_t<Range>;
        Fp sum = 0.0;
        Fp c = 0.0;
        for (const auto value : values) {
            Fp y = value - c;
            volatile const Fp t = sum + y;
            volatile const Fp z = t - sum;
            c = z - y;
            sum = t;
        };

        return sum;
    }
};
}  // namespace palloc

#endif