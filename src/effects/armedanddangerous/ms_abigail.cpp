#include "ms_abigail.h"

#include "effects/base/escapable.h"
#include "effects/base/draw_check.h"

#include "cards/game_enums.h"

#include "game/game_table.h"

namespace banggame {

    bool effect_ms_abigail::can_escape(player_ptr origin, card_ptr origin_card, effect_flags flags) {
        return origin && origin_card
            && origin_card->is_brown()
            && flags.check(effect_flag::single_target)
            && get_modified_sign(origin_card).is_jack_to_ace();
    }

    void equip_ms_abigail::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::apply_escapable_modifier>(origin_card,
            [=](card_ptr e_origin_card, player_ptr e_origin, const_player_ptr e_target, effect_flags e_flags, const escapable_request &req) {
                if (e_target == origin && effect_ms_abigail::can_escape(e_origin, e_origin_card, e_flags)) {
                    return escape_type::escape_no_timer;
                }
                return escape_type::no_escape;
            });
    }

    bool effect_ms_abigail::can_play(card_ptr origin_card, player_ptr origin) {
        if (auto req = origin->m_game->top_request<escapable_request>([&](const request_base &base) {
            return base.target == origin && effect_ms_abigail::can_escape(base.origin, base.origin_card, base.flags);
        })) {
            return req->can_escape(origin_card);
        }
        return false;
    }

    prompt_string effect_ms_abigail::on_prompt(card_ptr origin_card, player_ptr origin) {
        auto req = origin->m_game->top_request<escapable_request>();
        return req->escape_prompt(origin);
    }

    void effect_ms_abigail::on_play(card_ptr origin_card, player_ptr origin) {
        auto req = origin->m_game->top_request<escapable_request>();
        req->add_card(origin_card);
        
        origin_card->flash_card();
        origin->m_game->pop_request();
    }
}