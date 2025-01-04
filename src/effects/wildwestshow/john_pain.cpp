#include "john_pain.h"

#include "effects/base/vulture_sam.h"
#include "effects/base/draw_check.h"

#include "game/game.h"

namespace banggame {

    static card_ptr get_john_pain(player_ptr target) {
        card_ptr origin_card = nullptr;
        target->m_game->call_event(event_type::check_card_taker{ target, card_taker_type::draw_check_select, origin_card });
        return origin_card;
    }
    
    void equip_john_pain::on_enable(card_ptr target_card, player_ptr player_end) {
        player_end->m_game->add_listener<event_type::check_card_taker>(target_card, [=](player_ptr e_target, card_taker_type type, card_ptr &value) {
            if (type == card_taker_type::draw_check_select && e_target == player_end && e_target->m_hand.size() < 6) {
                value = target_card;
            }
        });
        player_end->m_game->add_listener<event_type::on_draw_check_resolve>(target_card, [=](card_ptr origin_card, player_ptr player_begin, card_ptr target_card, card_ptr drawn_card) {
            player_end->m_game->queue_action([=]{
                if (player_end->alive() && target_card->pocket != pocket_type::player_hand && get_john_pain(player_end)) {
                    if (rn::none_of(player_begin->m_game->range_all_players(player_begin)
                        | rv::take_while([=](const_player_ptr current) { return current != player_end; })
                        | rv::filter(&player::alive),
                        get_john_pain)
                    ) {
                        player_end->m_game->add_log("LOG_DRAWN_CARD", player_end, target_card);
                        player_end->add_to_hand(target_card);
                    }
                }
            });
        });
    }
}