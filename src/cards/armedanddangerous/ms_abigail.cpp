#include "ms_abigail.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    static bool ms_abigail_can_escape(player *origin, card *origin_card, effect_flags flags) {
        if (!origin) return false;
        if (!bool(flags & effect_flags::single_target)) return false;
        if (!origin_card->is_brown()) return false;
        switch (origin->m_game->get_card_sign(origin_card).rank) {
        case card_rank::rank_J:
        case card_rank::rank_Q:
        case card_rank::rank_K:
        case card_rank::rank_A:
            return true;
        default:
            return false;
        }
    }

    void equip_ms_abigail::on_enable(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::apply_escapable_modifier>({origin_card, -1},
            [=](card *e_origin_card, player *e_origin, const player *e_target, effect_flags e_flags, int &value) {
                if (e_target == origin && ms_abigail_can_escape(e_origin, e_origin_card, e_flags)) {
                    value = 2;
                }
            });
    }

    bool effect_ms_abigail::can_play(card *origin_card, player *origin) {
        if (auto req = origin->m_game->top_request(origin)) {
            return ms_abigail_can_escape(req->origin, req->origin_card, req->flags);
        }
        return false;
    }

    void effect_ms_abigail::on_play(card *origin_card, player *origin) {
        origin->m_game->flash_card(origin_card);
        origin->m_game->pop_request();
    }
}