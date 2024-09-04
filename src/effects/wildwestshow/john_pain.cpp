#include "john_pain.h"

#include "effects/base/vulture_sam.h"
#include "effects/base/draw_check.h"

#include "game/game.h"

namespace banggame {

    static card_ptr get_john_pain(player_ptr target) {
        card_ptr origin_card = nullptr;
        target->m_game->call_event(event_type::check_card_taker{ target, card_taker_type::draw_checks, origin_card });
        return origin_card;
    }
    
    void equip_john_pain::on_enable(card_ptr target_card, player_ptr player_end) {
        player_end->m_game->add_listener<event_type::check_card_taker>(target_card, [=](player_ptr e_target, card_taker_type type, card* &value) {
            if (type == card_taker_type::draw_checks && e_target == player_end && e_target->m_hand.size() < 6) {
                value = target_card;
            }
        });
        player_end->m_game->add_listener<event_type::on_draw_check_resolve>(target_card, [=](player_ptr player_begin, card_ptr drawn_card) {
            player_end->m_game->queue_action([=]{
                if (player_end->alive() && drawn_card->pocket != pocket_type::player_hand
                    && std::none_of(player_iterator(player_begin), player_iterator(player_end), get_john_pain)
                    && get_john_pain(player_end))
                {
                    player_end->m_game->add_log("LOG_DRAWN_CARD", player_end, drawn_card);
                    player_end->add_to_hand(drawn_card);
                }
            });
        });
    }
}