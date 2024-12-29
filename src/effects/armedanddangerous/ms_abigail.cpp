#include "ms_abigail.h"

#include "effects/base/resolve.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    bool effect_ms_abigail::can_escape(player_ptr origin, card_ptr origin_card, effect_flags flags) {
        if (origin && origin_card
            && origin_card->is_brown()
            && flags.check(effect_flag::single_target)
        ) {
            auto rank = origin_card->get_modified_sign().rank;
            return enums::indexof(rank) >= enums::indexof(card_rank::rank_J)
                && enums::indexof(rank) <= enums::indexof(card_rank::rank_A);
        }
        return false;
    }

    void equip_ms_abigail::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::apply_escapable_modifier>({origin_card, -1},
            [=](card_ptr e_origin_card, player_ptr e_origin, const_player_ptr e_target, effect_flags e_flags, escape_type &value) {
                if (e_target == origin && effect_ms_abigail::can_escape(e_origin, e_origin_card, e_flags)) {
                    value = escape_type::escape_no_timer;
                }
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

    void effect_ms_abigail::on_play(card_ptr origin_card, player_ptr origin) {
        auto req = origin->m_game->top_request<escapable_request>();
        req->add_card(origin_card);
        
        origin_card->flash_card();
        origin->m_game->pop_request();
    }
}