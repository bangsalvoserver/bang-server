#ifndef __PLAYER_ITERATOR_H__
#define __PLAYER_ITERATOR_H__

#include "player.h"

namespace banggame {

    struct player_iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = int;
        using value_type = player_ptr;
        using pointer = value_type *;
        using reference = value_type &;

        player_iterator() = default;

        explicit player_iterator(const_player_ptr p)
            : m_list{&p->get_all_players()}
            , m_it{rn::find(*m_list, p)}
        {
            if (m_it == m_list->end()) {
                throw game_error("Invalid player_iterator");
            }
        }

        reference operator *() const { return *m_it; }
        pointer operator ->() { return &*m_it; }

        player_iterator &operator++() {
            auto start = m_it;
            while (true) {
                ++m_it;
                if (m_it == m_list->end()) {
                    m_it = m_list->begin();
                }
                if ((*m_it)->alive()) {
                    break;
                }
                if (m_it == start) {
                    throw game_error("Infinite loop in player_iterator");
                }
            }
            return *this;
        }

        player_iterator operator++(int) {
            auto copy = *this;
            ++*this;
            return copy;
        }

        player_iterator &operator--() {
            auto start = m_it;
            while (true) {
                if (m_it == m_list->begin()) {
                    m_it = m_list->end();
                }
                --m_it;
                if ((*m_it)->alive()) {
                    break;
                }
                if (m_it == start) {
                    throw game_error("Infinite loop in player_iterator");
                }
            }
            return *this;
        }

        player_iterator operator--(int) {
            auto copy = *this;
            --*this;
            return copy;
        }

        bool operator == (const player_iterator &other) const = default;

    private:
        player_list *m_list;
        player_list::iterator m_it;
    };

    inline auto range_all_players(player_ptr begin) {
        auto &list = begin->get_all_players();
        auto it = rn::find(list, begin);
        return rv::concat(rn::subrange(it, list.end()), rn::subrange(list.begin(), it));
    }

    inline auto range_alive_players(player_ptr begin) {
        return range_all_players(begin) | rv::filter(&player::alive);
    }

    inline auto range_other_players(player_ptr begin) {
        return range_all_players(begin) | rv::drop(1) | rv::filter(&player::alive);
    }

}

#endif