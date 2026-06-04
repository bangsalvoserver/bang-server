#include "john_pain.h"

#include "effects/base/vulture_sam.h"
#include "effects/base/draw_check.h"

#include "game/game_table.h"

namespace banggame {

    static card_ptr get_john_pain(player_ptr target) {
        return target->m_game->call_event(event_type::check_card_taker{ target, card_taker_type::draw_check_select });
    }
    
    void equip_john_pain::on_enable(card_ptr target_card, player_ptr player_end) {
        player_end->m_game->add_listener<event_type::check_card_taker>(target_card, [=](player_ptr e_target, card_taker_type type) -> card_ptr {
            if (type == card_taker_type::draw_check_select && e_target == player_end) {
                return target_card;
            }
            return nullptr;
        });
        player_end->m_game->add_listener<event_type::on_draw_check_resolve>({ target_card, 1 }, [=](card_ptr origin_card, player_ptr player_begin, card_ptr target_card, card_ptr drawn_card) {
            int priority = 30;
            for (player_ptr current : player_begin->m_game->range_all_players(player_begin)) {
                if (current == player_end) break;
                if (current->alive() && get_john_pain(current)) --priority;
            }
            
            player_end->m_game->queue_action([=]{
                player_end->m_game->queue_action([=]{
                    if (player_end->alive() && target_card->pocket != pocket_type::player_hand && player_end->m_hand.size() < 6) {
                        player_end->m_game->add_log("LOG_DRAWN_CARD", player_end, target_card);
                        target_card->add_short_pause();
                        player_end->add_to_hand(target_card);
                    }
                });
            }, priority);
        });
    }
}