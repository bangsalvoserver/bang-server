#ifndef __RANGE_UTILS_H__
#define __RANGE_UTILS_H__

#ifdef USE_STD_RANGES

#include <ranges>

namespace rn = std::ranges;
namespace rv = std::views;

#else

#include <range/v3/all.hpp>

namespace rn = ranges;
namespace rv = ranges::views;

#endif

#include <stdexcept>

template<rn::input_range R> requires std::is_pointer_v<rn::range_value_t<R>>
rn::range_value_t<R> get_single_element(R &&range) {
    auto begin = rn::begin(range);
    auto end = rn::end(range);

    if (begin != end) {
        auto first = *begin;
        if (++begin == end) {
            return first;
        }
    }
    return nullptr;
}

template<rn::input_range R>
bool contains_at_least(R &&range, int size) {
    if (size == 0) return true;
    for (const auto &value : range) {
        if (--size == 0) return true;
    }
    return false;
}

struct random_element_error : std::runtime_error {
    random_element_error(): std::runtime_error{"Empty range in random_element"} {}
};

template<rn::forward_range R, typename Rng>
decltype(auto) random_element(R &&range, Rng &rng) {
    rn::range_value_t<R> ret;
    if (rn::sample(std::forward<R>(range), &ret, 1, rng).out == &ret) {
        throw random_element_error();
    }
    return ret;
}

template<rn::range Rng>
inline auto rotate_range(Rng &&rng, rn::iterator_t<Rng> it) {
    return rv::concat(rn::subrange(it, rn::end(rng)), rn::subrange(rn::begin(rng), it));
}

inline bool string_equal_icase(std::string_view lhs, std::string_view rhs) {
    return rn::equal(lhs, rhs, {}, toupper, toupper);
}

#endif