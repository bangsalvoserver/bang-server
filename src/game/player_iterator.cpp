#include "player_iterator.h"

#include "game.h"
#include "player.h"

namespace banggame {
    player_iterator::player_iterator(player *p)
        : m_it(std::ranges::find(p->m_game->m_players, p)) {}

    player_iterator &player_iterator::operator++() {
        auto &list = (*m_it)->m_game->m_players;
        auto start = m_it;
        while (true) {
            ++m_it;
            if (m_it == list.end()) {
                m_it = list.begin();
            }
            if ((*m_it)->alive()) {
                break;
            }
            if (m_it == start) {
                throw std::runtime_error("Infinite loop in player_iterator");
            }
        }
        return *this;
    }

    player_iterator &player_iterator::operator--() {
        auto &list = (*m_it)->m_game->m_players;
        auto start = m_it;
        while (true) {
            if (m_it == list.begin()) {
                m_it = list.end();
            }
            --m_it;
            if ((*m_it)->alive()) {
                break;
            }
            if (m_it == start) {
                throw std::runtime_error("Infinite loop in player_iterator");
            }
        }
        return *this;
    }

    util::generator<player *> range_all_players(player *begin) {
        auto &list = begin->m_game->m_players;
        auto it = std::ranges::find(list, begin);
        while (true) {
            if ((*it)->alive()) {
                co_yield *it;
            }
            if (++it == list.end()) {
                it = list.begin();
            }
            if (*it == begin) {
                break;
            }
        }
    }

    util::generator<player *> range_all_players_and_dead(player *begin) {
        auto &list = begin->m_game->m_players;
        auto it = std::ranges::find(list, begin);
        while (true) {
            co_yield *it;
            if (++it == list.end()) {
                it = list.begin();
            }
            if (*it == begin) {
                break;
            }
        }
    }

    util::generator<player *> range_other_players(player *begin) {
        auto &list = begin->m_game->m_players;
        auto it = std::ranges::find(list, begin);
        while (true) {
            if (++it == list.end()) {
                it = list.begin();
            }
            if (*it == begin) {
                break;
            }
            if ((*it)->alive()) {
                co_yield *it;
            }
        }
    }
}