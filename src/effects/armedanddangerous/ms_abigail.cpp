#include "ms_abigail.h"

#include "effects/base/resolve.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    static bool ms_abigail_can_escape(player_ptr origin, card_ptr origin_card, effect_flags flags) {
        if (!origin) return false;
        if (!flags.check(effect_flag::single_target)) return false;
        if (!origin_card || !origin_card->is_brown()) return false;
        switch (origin_card->get_modified_sign().rank) {
        case card_rank::rank_J:
        case card_rank::rank_Q:
        case card_rank::rank_K:
        case card_rank::rank_A:
            return true;
        default:
            return false;
        }
    }

    void equip_ms_abigail::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::apply_escapable_modifier>({origin_card, -1},
            [=](card_ptr e_origin_card, player_ptr e_origin, const_player_ptr e_target, effect_flags e_flags, int &value) {
                if (e_target == origin && ms_abigail_can_escape(e_origin, e_origin_card, e_flags)) {
                    value = 2;
                }
            });
    }

    bool effect_ms_abigail::can_play(card_ptr origin_card, player_ptr origin) {
        if (auto req = origin->m_game->top_request(origin)) {
            if (auto req2 = std::dynamic_pointer_cast<escapable_request>(req)) {
                return req2->can_escape(origin_card)
                    && ms_abigail_can_escape(req->origin, req->origin_card, req->flags);
            }
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