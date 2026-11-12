#include "fishing.h"

#include "game/game_table.h"

#include "effects/base/draw_check.h"

namespace banggame {

    int effect_fishing::evaluate(card_ptr, player_ptr origin) const {
        card_suit suit_mod = origin->m_game->call_event(event_type::get_suit_modifier{});
        if (suit_mod != card_suit::none && suit_mod != suit) {
            return -5;
        }
        return 0;
    }

    void effect_fishing::on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) const {
        for (player_ptr p : origin->m_game->m_players) {
            if (p != origin && p->alive()) {
                origin->m_game->queue_action([=, suit=suit]{
                    origin->m_game->queue_request<request_check>(origin, origin_card,
                        [=](card_sign s) { return s.suit == suit; },
                        [=](card_ptr drawn_card, card_sign drawn_sign, bool lucky) {
                            if (lucky && drawn_card->pocket != pocket_type::player_hand) {
                                origin->m_game->add_log("LOG_DRAWN_CARD", origin, drawn_card);
                                drawn_card->add_short_pause();
                                origin->add_to_hand(drawn_card);
                            }
                        }
                    );
                }, -1);
            }
        }
    }
}
