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
    using value_type = player_ptr;
    using pointer = value_type *;
    using reference = value_type &;

    player_iterator() = default;
    explicit player_iterator(const_player_ptr p);

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

utils::generator<player_ptr> range_all_players(const_player_ptr begin);
utils::generator<player_ptr> range_all_players_and_dead(const_player_ptr begin);
utils::generator<player_ptr> range_other_players(const_player_ptr begin);

}

#endif