#ifndef __DISABLER_MAP_H__
#define __DISABLER_MAP_H__

#include <functional>
#include <map>

#include "event_card_key.h"

namespace banggame {

    struct game_table;

    using card_disabler_fun = std::function<bool(card *)>;

    class disabler_map {
    private:
        std::multimap<event_card_key, card_disabler_fun, std::less<>> m_disablers;
        game_table *m_game;

    public:
        disabler_map(game_table *game): m_game(game) {}

        void add_disabler(event_card_key key, card_disabler_fun &&fun);
        void remove_disablers(event_card_key key);

        card *get_disabler(card *target_card) const;
        bool is_disabled(card *target_card) const;
    };
}

#endif