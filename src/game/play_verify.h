#ifndef __PLAY_VERIFY_H__
#define __PLAY_VERIFY_H__

#include <variant>
#include <vector>
#include <set>

#include "card_enums.h"
#include "holders.h"
#include "filters.h"

namespace banggame {

    struct play_card_verify {
        player *origin;
        card *origin_card;
        bool is_response;
        target_list targets;
        std::vector<card *> modifiers;

        game_string verify_modifiers() const;
        game_string verify_duplicates() const;
        game_string verify_equip_target() const;
        game_string verify_card_targets() const;

        game_string check_prompt() const;
        game_string check_prompt_equip() const;

        player *get_equip_target() const;
        
        void play_modifiers() const;
        void do_play_card() const;

        game_string verify_and_play();
    };

    struct duplicate_sets {
        std::set<player *> players;
        std::set<card *> cards;
        std::map<card *, int> cubes;
    };

}

#endif