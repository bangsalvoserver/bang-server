#ifndef __GREATTRAINROBBERY_SGT_BLAZE_H__
#define __GREATTRAINROBBERY_SGT_BLAZE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct modifier_sgt_blaze {
        bool valid_with_equip(card_ptr origin_card, player_ptr origin, card_ptr playing_card) const {
            return false;
        }
        bool valid_with_modifier(card_ptr origin_card, player_ptr origin, card_ptr playing_card) const {
            return true;
        }
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr playing_card, const effect_context &ctx) const;
    };
    
    DEFINE_MODIFIER(sgt_blaze, modifier_sgt_blaze)

    struct effect_skip_player {
        void add_context(card_ptr origin_card, player_ptr origin, player_ptr target, effect_context &ctx) const;
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) const;
    };

    DEFINE_EFFECT(skip_player, effect_skip_player)

    struct equip_sgt_blaze : event_equip {
        void on_enable(card_ptr origin_card, player_ptr origin) const;
    };

    DEFINE_EQUIP(sgt_blaze, equip_sgt_blaze)

    namespace contexts {
        struct skipped_player {
            player_ptr value;
        };
    }
    
}

#endif