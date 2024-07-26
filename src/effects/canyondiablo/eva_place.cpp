#include "eva_place.h"

#include "game/game.h"

#include "cards/game_enums.h"

namespace banggame {

    void effect_eva_place::on_play(card_ptr origin_card, player_ptr origin) {
        card_ptr drawn_card = origin->m_game->top_of_deck();
        origin->m_game->add_log("LOG_DRAWN_CARD_FOR", origin, drawn_card, origin_card);
        drawn_card->set_visibility(card_visibility::shown);
        drawn_card->add_short_pause();
        origin->add_to_hand(drawn_card);
        if (drawn_card->sign.is_diamonds()) {
            origin->draw_card(1, origin_card);
        }
    }
}