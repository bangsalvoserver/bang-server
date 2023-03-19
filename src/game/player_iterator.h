#ifndef __PLAYER_ITERATOR_H__
#define __PLAYER_ITERATOR_H__

#include <ranges>

#include "game_table.h"
#include "utils/generator.h"

namespace banggame {

struct player_iterator {
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = int;
    using value_type = player *;
    using pointer = value_type *;
    using reference = value_type &;

    player_iterator() = default;
    explicit player_iterator(player *p);

    reference operator *() const { return *m_it; }
    pointer operator ->() { return &*m_it; }

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

private:
    decltype(game_table::m_players)::iterator m_it;
};

util::generator<player *> range_all_players(player *begin);
util::generator<player *> range_all_players_and_dead(player *begin);
util::generator<player *> range_other_players(player *begin);

}

#endif