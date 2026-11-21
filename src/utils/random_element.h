#ifndef __RANDOM_ELEMENT_H__
#define __RANDOM_ELEMENT_H__

#include "range_utils.h"

template<rn::input_range R, typename Rng>
std::vector<rn::range_value_t<R>> sample_elements(R &&range, size_t k, Rng &rng) {
    using value_t = rn::range_value_t<R>;

    std::vector<value_t> result;

    if (k != 0) {   
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
    }

    return result;
}

template<rn::input_range R, typename Rng>
rn::range_value_t<R> random_element(R &&range, Rng &rng) {
    using value_t = rn::range_value_t<R>;

    if constexpr (rn::sized_range<R>) {
        auto n = rn::size(range);
        if (n == 0) throw std::runtime_error("Empty range in random_element");

        std::uniform_int_distribution<size_t> dist(0, n - 1);
        auto it = rn::next(rn::begin(range), dist(rng));
        return *it;
    } else {
        auto it = rn::begin(range);
        auto end = rn::end(range);
        if (it == end) throw std::runtime_error("Empty range in random_element");

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

// Returns a uniformly random element among those with the highest score,
// together with that score. Single pass, no allocation.
template<rn::input_range R, typename Rng, typename ScoreFn>
std::pair<rn::range_value_t<R>, int> random_max_element(R &&range, ScoreFn &&score_fn, Rng &rng) {
    using value_t = rn::range_value_t<R>;

    std::optional<value_t> chosen;
    int best = 0;
    size_t ties = 0;

    for (auto &&elem : range) {
        int score = score_fn(elem);
        if (!chosen || score > best) {
            // new maximum: restart the tie counter
            chosen = elem;
            best = score;
            ties = 1;
        } else if (score == best) {
            // reservoir sampling: the k-th element tied for the maximum
            // replaces the current choice with probability 1/k
            ++ties;
            std::uniform_int_distribution<size_t> dist(0, ties - 1);
            if (dist(rng) == 0) chosen = elem;
        }
    }

    if (!chosen) throw std::runtime_error("Empty range in random_max_element");
    return { std::move(*chosen), best };
}

// Returns the `count` highest scored elements of `range`, together
// with the sum of their scores. Ties are broken uniformly at random.
// The range is traversed exactly once and score_fn is called once per element.
template<rn::input_range R, typename ScoreFn, typename Rng>
std::pair<std::vector<rn::range_value_t<R>>, int> random_max_elements(R &&range, size_t count, ScoreFn &&score_fn, Rng &rng) {
    using value_t = rn::range_value_t<R>;

    struct entry {
        int score;
        uint32_t tiebreak;
        value_t value;

        auto operator < (const entry &rhs) const {
            return std::tie(rhs.score, rhs.tiebreak) < std::tie(score, tiebreak);
        }
    };

    std::vector<entry> top;
    top.reserve(std::min<size_t>(count, 16));

    for (auto &&elem : range) {
        entry cand{ score_fn(elem), static_cast<uint32_t>(rng()), value_t(elem) };

        if (top.size() < count) {
            top.push_back(std::move(cand));
            std::push_heap(top.begin(), top.end());
        } else if (cand < top.front()) {
            std::pop_heap(top.begin(), top.end());
            top.back() = std::move(cand);
            std::push_heap(top.begin(), top.end());
        }
    }

    std::vector<value_t> selected;
    selected.reserve(top.size());
    int total_score = 0;
    for (entry &e : top) {
        total_score += e.score;
        selected.push_back(std::move(e.value));
    }
    return { std::move(selected), total_score };
}

#endif
