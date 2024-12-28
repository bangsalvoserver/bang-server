#include "mick_defender.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    void equip_mick_defender::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::apply_escapable_modifier>({origin_card, -1},
            [=](card_ptr e_origin_card, player_ptr e_origin, const_player_ptr e_target, effect_flags flags, escape_type &value) {
                if (e_target == origin && !origin->empty_hand()
                    && e_origin_card && e_origin_card->is_brown()
                    && !flags.check(effect_flag::is_bang)
                ) {
                    value = escape_type::escape_no_timer;
                }
            });
    }

    void equip_mick_defender2::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::apply_escapable_modifier>({origin_card, -1},
            [=](card_ptr e_origin_card, player_ptr e_origin, const_player_ptr e_target, effect_flags flags, escape_type &value) {
                if (e_target == origin && !origin->empty_hand()
                    && e_origin_card && e_origin_card->is_brown()
                    && !flags.check(effect_flag::is_bang)
                    && flags.check(effect_flag::single_target)
                    && !flags.check(effect_flag::multi_target)
                ) {
                    value = escape_type::escape_no_timer;
                }
            });
    }
}