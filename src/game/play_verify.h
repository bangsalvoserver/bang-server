#ifndef __PLAY_VERIFY_H__
#define __PLAY_VERIFY_H__

#include <variant>
#include <vector>

#include "card_enums.h"
#include "holders.h"

namespace banggame {

    game_string check_player_filter(card *origin_card, player *origin, target_player_filter filter, player *target);
    game_string check_card_filter(card *origin_card, player *origin, target_card_filter filter, card *target);

    struct play_card_verify {
        player *origin;
        card *card_ptr;
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
        game_string verify_and_respond();
    };

}

#endif