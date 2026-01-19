#include "jourdonnais.h"

#include "effects/base/escapable.h"
#include "effects/base/draw_check.h"

#include "cards/game_enums.h"

#include "game/game_table.h"

namespace banggame {

    bool effect_jourdonnais_legend::can_escape(player_ptr origin, card_ptr origin_card, effect_flags flags) {
        return origin && origin_card && origin_card->is_brown();
    }

    void equip_jourdonnais_legend::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::apply_escapable_modifier>(origin_card,
            [=](card_ptr e_origin_card, player_ptr e_origin, const_player_ptr e_target, effect_flags e_flags, const escapable_request &req) {
                if (e_target == origin && effect_jourdonnais_legend::can_escape(e_origin, e_origin_card, e_flags) && req.can_escape(origin_card)) {
                    return escape_type::escape_no_timer;
                }
                return escape_type::no_escape;
            });
    }

    bool effect_jourdonnais_legend::can_play(card_ptr origin_card, player_ptr origin) {
        if (auto req = origin->m_game->top_request<escapable_request>([&](const request_base &base) {
            return base.target == origin && effect_jourdonnais_legend::can_escape(base.origin, base.origin_card, base.flags);
        })) {
            return req->can_escape(origin_card);
        }
        return false;
    }

    prompt_string effect_jourdonnais_legend::on_prompt(card_ptr origin_card, player_ptr origin) {
        auto req = origin->m_game->top_request<escapable_request>();
        return req->escape_prompt(origin);
    }

    void effect_jourdonnais_legend::on_play(card_ptr origin_card, player_ptr origin) {
        auto req = origin->m_game->top_request<escapable_request>();
        req->add_card(origin_card);

        origin->m_game->queue_request<request_check>(origin, origin_card, &card_sign::is_jack_to_ace, [=](bool result) {
            if (result) {
                origin->m_game->add_log("LOG_CARD_HAS_EFFECT", origin_card);
                origin->m_game->pop_request();
            }
        });
    }
}