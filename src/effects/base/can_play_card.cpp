#include "can_play_card.h"

#include "game/game_table.h"

namespace banggame {

    void request_can_play_card::on_resolve() {
        target->m_game->pop_request();
    }
    
    game_string request_can_play_card::status_text(player_ptr owner) const {
        if (target == owner) {
            return {"STATUS_CAN_PLAY_CARD", origin_card};
        } else {
            return {"STATUS_CAN_PLAY_CARD_OTHER", target, origin_card};
        }
    }

    bool effect_can_play_card::can_play(card_ptr origin_card, player_ptr origin) {
        if (auto req = origin->m_game->top_request<request_can_play_card>(target_is{origin})) {
            return req->origin_card == origin_card;
        }
        return false;
    }

    void effect_can_play_card::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->pop_request();
    }
}
