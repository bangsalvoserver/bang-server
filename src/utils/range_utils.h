#ifndef __RANGE_UTILS_H__
#define __RANGE_UTILS_H__

#include <ranges>
#include <algorithm>
#include <stdexcept>

#if !defined(__cpp_lib_ranges_concat)
#include "ranges_concat.h"
#endif

namespace rn = std::ranges;
namespace rv = std::views;

namespace std::ranges::views {

    template<typename T>
    auto for_each(T &&fn) {
        return rv::transform(std::forward<T>(fn)) | rv::join;
    }

    template<typename T>
    auto take_last(T &&n) {
        return rv::reverse | rv::take(std::forward<T>(n)) | rv::reverse;
    }

    template<typename T>
    auto remove_if(T &&fn) {
        return rv::filter(std::not_fn(std::forward<T>(fn)));
    }

    inline constexpr auto addressof = rv::transform([](auto &&value) {
        return &value;
    });

}

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

template<rn::range Rng>
inline auto rotate_range(Rng &&rng, rn::iterator_t<Rng> it) {
    return rv::concat(rn::subrange(it, rn::end(rng)), rn::subrange(rn::begin(rng), it));
}

inline bool string_equal_icase(std::string_view lhs, std::string_view rhs) {
    return rn::equal(lhs, rhs, {}, toupper, toupper);
}

template<rn::forward_range auto Range, auto Pred>
    requires std::predicate<decltype(Pred), rn::range_reference_t<decltype(Range)>>
constexpr auto filter_static_array() {
    using value_type = rn::range_value_t<decltype(Range)>;

    static constexpr size_t count = []{
        size_t count = 0;
        for (auto &&value : Range) {
            if (std::invoke(Pred, value)) ++count;
        }
        return count;
    }();

    std::array<value_type, count> result;

    auto it = result.begin();
    for (auto &&value : Range) {
        if (std::invoke(Pred, value)) {
            *it = std::move(value);
            ++it;
        }
    }
    return result;
}

#endif