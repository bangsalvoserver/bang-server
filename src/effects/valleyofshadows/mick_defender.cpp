#include "mick_defender.h"

#include "escape.h"

#include "cards/game_enums.h"

#include "game/game_table.h"

namespace banggame {

    void equip_mick_defender::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::apply_escapable_modifier>({origin_card, -1},
            [=](card_ptr e_origin_card, player_ptr e_origin, const_player_ptr e_target, effect_flags flags, escape_type &value) {
                if (e_target == origin && !origin->empty_hand()
                    && effect_escape::can_escape(e_origin, e_origin_card, flags)
                ) {
                    value = escape_type::escape_no_timer;
                }
            });
    }

    void equip_mick_defender2::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::apply_escapable_modifier>({origin_card, -1},
            [=](card_ptr e_origin_card, player_ptr e_origin, const_player_ptr e_target, effect_flags flags, escape_type &value) {
                if (e_target == origin && !origin->empty_hand()
                    && effect_escape2::can_escape(e_origin, e_origin_card, flags)
                ) {
                    value = escape_type::escape_no_timer;
                }
            });
    }
}