#ifndef __PLAY_VERIFY_H__
#define __PLAY_VERIFY_H__

#include <variant>
#include <vector>

#include "card_enums.h"
#include "holders.h"

namespace banggame {

    opt_game_str check_player_filter(card *origin_card, player *origin, target_player_filter filter, player *target);
    opt_game_str check_card_filter(card *origin_card, player *origin, target_card_filter filter, card *target);

    struct play_card_verify {
        player *origin;
        card *card_ptr;
        bool is_response;
        target_list targets;
        std::vector<card *> modifiers;

        [[nodiscard]] opt_game_str verify_modifiers() const;
        [[nodiscard]] opt_game_str verify_equip_target() const;
        [[nodiscard]] opt_game_str verify_card_targets() const;

        opt_game_str check_prompt() const;
        opt_game_str check_prompt_equip() const;

        player *get_equip_target() const;
        
        void play_modifiers() const;
        void do_play_card() const;

        [[nodiscard]] opt_game_str verify_and_play();
        [[nodiscard]] opt_game_str verify_and_respond();
    };

}

#endif