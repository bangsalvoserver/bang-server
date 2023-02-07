#ifndef __PLAYER_ITERATOR_H__
#define __PLAYER_ITERATOR_H__

#include <ranges>

#include "game_table.h"

namespace banggame {

class player_iterator_base {
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = int;
    using value_type = player *;
    using pointer = value_type *;
    using reference = value_type &;

    player_iterator_base() = default;
    explicit player_iterator_base(player *p);

    reference operator *() const { return *m_it; }
    pointer operator ->() { return &*m_it; }

    bool operator == (const player_iterator_base &other) const = default;

protected:
    decltype(game_table::m_players)::iterator m_it;
};

struct player_iterator : player_iterator_base {
    using player_iterator_base::player_iterator_base;

    player_iterator &operator++();

    player_iterator operator++(int) {
        auto copy = *this;
        ++*this;
        return copy;
    }

    player_iterator &operator--();

    player_iterator operator--(int) {
        auto copy = *this;
        --*this;
        return copy;
    }

    bool operator == (const player_iterator &other) const = default;
};

class cycle_player_iterator : public player_iterator_base {
public:
    cycle_player_iterator() = default;

    explicit cycle_player_iterator(player *p, int cycle = 0, bool no_skip_dead = false)
        : player_iterator_base(p), m_cycle(cycle), m_no_skip_dead(no_skip_dead) {}

    cycle_player_iterator &operator++();

    cycle_player_iterator operator++(int) {
        auto copy = *this;
        ++*this;
        return copy;
    }

    cycle_player_iterator &operator--();

    cycle_player_iterator operator--(int) {
        auto copy = *this;
        --*this;
        return copy;
    }

    bool operator == (const cycle_player_iterator &other) const = default;

private:
    int m_cycle = 0;
    bool m_no_skip_dead = false;
};

inline player *first_alive_player(player *p) {
    if (p->alive()) {
        return p;
    } else {
        return *std::next(player_iterator(p));
    }
}

inline auto range_all_players(player *p, int cycles = 1, bool no_skip_dead = false) {
    player *begin = first_alive_player(p);
    return ranges::subrange(cycle_player_iterator(begin, 0, no_skip_dead), cycle_player_iterator(begin, cycles, no_skip_dead));
}

inline auto range_other_players(player *p) {
    player *begin = first_alive_player(p);
    return ranges::subrange(std::next(player_iterator(begin)), player_iterator(begin));
}

}

#endif