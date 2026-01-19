#include "mick_defender.h"

#include "escape.h"

#include "effects/base/escapable.h"

#include "game/game_table.h"

namespace banggame {

    void equip_mick_defender::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::apply_escapable_modifier>(origin_card,
            [=](card_ptr e_origin_card, player_ptr e_origin, const_player_ptr e_target, effect_flags flags, const escapable_request &req) {
                if (e_target == origin && !origin->empty_hand()
                    && effect_escape::can_escape(e_origin, e_origin_card, flags)
                ) {
                    return escape_type::escape_no_timer;
                }
                return escape_type::no_escape;
            });
    }

    void equip_mick_defender2::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::apply_escapable_modifier>(origin_card,
            [=](card_ptr e_origin_card, player_ptr e_origin, const_player_ptr e_target, effect_flags flags, const escapable_request &req) {
                if (e_target == origin && !origin->empty_hand()
                    && effect_escape2::can_escape(e_origin, e_origin_card, flags)
                ) {
                    return escape_type::escape_no_timer;
                }
                return escape_type::no_escape;
            });
    }
}