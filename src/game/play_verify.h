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
        card *playing_card;
        bool is_response;
        target_list targets;
        std::vector<modifier_pair> modifiers;

        play_card_verify() = default;
        play_card_verify(player *origin, card *origin_card, bool is_response = false, target_list targets = {}, std::vector<modifier_pair> modifiers = {});

        std::vector<card *> modifier_cards() const;

        game_string verify_modifiers_add_context(effect_context &ctx) const;
        game_string verify_duplicates() const;
        game_string verify_equip_target() const;
        game_string verify_card_targets_add_context(effect_context &ctx) const;

        game_string check_prompt(const effect_context &ctx = {}) const;
        game_string check_prompt_equip() const;

        player *get_equip_target() const;
        
        void play_modifiers(const effect_context &ctx = {}) const;
        void do_play_card(const effect_context &ctx = {}) const;

        game_string verify_and_play();
    };

}

#endif