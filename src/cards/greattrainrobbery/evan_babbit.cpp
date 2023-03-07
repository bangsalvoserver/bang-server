#include "evan_babbit.h"

#include "cards/base/bang.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    game_string handler_evan_babbit::get_error(card *origin_card, player *origin, card *target_card, player *target_player) {
        if (auto req = origin->m_game->top_request<request_bang>(origin)) {
            if (req->num_cards_used()) {
                return "ERROR_INVALID_ACTION";
            }
            
            if (!bool(req->flags & effect_flags::is_bang) || !req->origin_card) {
                return "ERROR_INVALID_ACTION";
            }

            if (req->origin == target_player) {
                return "ERROR_CANNOT_TARGET_SHOOTER";
            }

            if (origin->m_game->get_card_sign(req->origin_card).suit != origin->m_game->get_card_sign(target_card).suit) {
                return {"ERROR_INVALID_SIGN", target_card};
            }

            return {};
        } else {
            return "ERROR_INVALID_ACTION";
        }
    }

    void handler_evan_babbit::on_play(card *origin_card, player *origin, card *target_card, player *target_player) {
        origin->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, origin, target_card);
        origin->discard_card(target_card);
        auto req = origin->m_game->top_request();
        origin->m_game->add_log("LOG_DEFLECTED_BANG_TO", origin_card, origin, req->origin_card, target_player);
        req->target = target_player;
    }
}