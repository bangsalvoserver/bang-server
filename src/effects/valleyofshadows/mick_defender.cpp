#include "mick_defender.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    void equip_mick_defender::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::apply_escapable_modifier>({origin_card, -1},
            [=](card_ptr e_origin_card, player_ptr e_origin, const_player_ptr e_target, effect_flags flags, int &value) {
                if (e_target == origin && !origin->empty_hand() && flags.check(effect_flag::escapable)) {
                    value = 2;
                }
            });
    }

    void equip_mick_defender2::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::apply_escapable_modifier>({origin_card, -1},
            [=](card_ptr e_origin_card, player_ptr e_origin, const_player_ptr e_target, effect_flags flags, int &value) {
                if (e_target == origin && !origin->empty_hand()
                    && flags.check(effect_flag::escapable)
                    && flags.check(effect_flag::single_target)
                    && !flags.check(effect_flag::multi_target)
                ) {
                    value = 2;
                }
            });
    }
}