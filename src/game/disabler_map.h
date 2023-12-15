#ifndef __DISABLER_MAP_H__
#define __DISABLER_MAP_H__

#include <functional>
#include <map>

#include "event_card_key.h"

namespace banggame {

    struct game_table;

    using card_disabler_fun = std::function<bool(card *)>;

    class disabler_map {
    public:
        using disabler_map_t = std::multimap<event_card_key, card_disabler_fun, std::less<>>;
        using disabler_map_iterator = disabler_map_t::const_iterator;
        using disabler_map_range = std::ranges::subrange<disabler_map_iterator>;

    private:
        disabler_map_t m_disablers;
        game_table *m_game;

        void do_remove_disablers(disabler_map_range range);

    public:
        disabler_map(game_table *game): m_game(game) {}

        disabler_map_iterator add_disabler(event_card_key key, card_disabler_fun &&fun);

        void remove_disabler(disabler_map_iterator it) {
            do_remove_disablers(disabler_map_range{it, std::next(it)});
        }

        void remove_disablers(event_card_key key) {
            auto [low, high] = m_disablers.equal_range(key);
            do_remove_disablers({low, high});
        }

        void remove_disablers(card *key) {
            auto [low, high] = m_disablers.equal_range(key);
            do_remove_disablers({low, high});
        }

        card *get_disabler(card *target_card) const;
        bool is_disabled(card *target_card) const;
    };
}

#endif