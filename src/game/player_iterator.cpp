#include "player_iterator.h"

#include "game.h"
#include "player.h"

namespace banggame {
    player_iterator_base::player_iterator_base(pointer p)
        : m_it(p->m_game->m_players.find(p->id)) {}

    player_iterator &player_iterator::operator++() {
        auto &list = m_it->m_game->m_players;
        do {
            ++m_it;
            if (m_it == list.end()) {
                m_it = list.begin();
            }
        } while(!m_it->alive());
        return *this;
    }

    player_iterator &player_iterator::operator--() {
        auto &list = m_it->m_game->m_players;
        do {
            if (m_it == list.begin()) {
                m_it = list.end();
            }
            --m_it;
        } while(!m_it->alive());
        return *this;
    }

    cycle_player_iterator &cycle_player_iterator::operator++() {
        auto &list = m_it->m_game->m_players;
        do {
            ++m_it;
            if (m_it == list.end()) {
                m_it = list.begin();
                ++m_cycle;
            }
        } while(!m_it->alive());
        return *this;
    }

    cycle_player_iterator &cycle_player_iterator::operator--() {
        auto &list = m_it->m_game->m_players;
        do {
            if (m_it == list.begin()) {
                m_it = list.end();
                --m_cycle;
            }
            --m_it;
        } while(!m_it->alive());
        return *this;
    }
}