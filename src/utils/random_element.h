#ifndef __RANDOM_ELEMENT_H__
#define __RANDOM_ELEMENT_H__

#include "range_utils.h"

template<typename T, rn::input_range R, typename Rng>
std::vector<T> sample_elements_r(R &&range, size_t k, Rng &rng) {
    std::vector<T> result;
    
    auto it = rn::begin(range);
    auto end = rn::end(range);
    
    if (it != end) {
        result.reserve(k);
        size_t count = 0;

        for (; it != end && count < k; ++it, ++count) {
            result.push_back(*it);
        }

        for (; it != end; ++it, ++count) {
            std::uniform_int_distribution<size_t> dist(0, count);
            size_t idx = dist(rng);
            if (idx < k) result[idx] = *it;
        }
    }

    return result;
}

template<rn::input_range R, typename Rng>
auto sample_elements(R &&range, size_t count, Rng &rng) {
    return sample_elements_r<rn::range_value_t<R>>(std::forward<R>(range), count, rng);
}

template<typename T, rn::input_range R, typename Rng>
std::vector<T> sample_elements_streaming_r(R &&range, double probability, Rng &rng) {
    std::vector<T> result;
    
    std::bernoulli_distribution dist(probability);

    std::optional<T> fallback;
    size_t count = 0;

    for (auto &&elem : range) {
        if (dist(rng)) result.push_back(elem);
        std::uniform_int_distribution<size_t> fallback_dist(0, count);
        if (fallback_dist(rng) == 0) fallback = elem;
        ++count;
    }

    if (result.empty() && fallback.has_value()) {
        result.emplace_back(std::move(*fallback));
    }

    return result;
}

template<rn::input_range R, typename Rng>
auto sample_elements_streaming(R &&range, double probability, Rng &rng) {
    return sample_elements_streaming_r<rn::range_value_t<R>>(std::forward<R>(range), probability, rng);
}

struct random_element_error : std::runtime_error {
    random_element_error(): std::runtime_error{"Empty range in random_element"} {}
};

template<rn::input_range R, typename Rng>
rn::range_value_t<R> random_element(R &&range, Rng &rng) {
    using value_t = rn::range_value_t<R>;

    if constexpr (rn::sized_range<R>) {
        auto n = rn::size(range);
        if (n == 0) throw random_element_error();

        std::uniform_int_distribution<size_t> dist(0, n - 1);
        auto it = rn::next(rn::begin(range), dist(rng));
        return *it;
    } else {
        auto it = rn::begin(range);
        auto end = rn::end(range);
        if (it == end) throw random_element_error();

        value_t chosen = *it;
        size_t count = 0;

        ++it;
        for (; it != end; ++it) {
            ++count;
            std::uniform_int_distribution<size_t> dist(0, count);
            if (dist(rng) == 0) chosen = *it;
        }

        return chosen;
    }
}

#endif