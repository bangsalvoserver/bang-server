#include "gary_looter.h"

#include "game/game_table.h"

#include "effects/base/vulture_sam.h"
#include "effects/base/requests.h"

namespace banggame {

    static card_ptr get_gary_looter(player_ptr target) {
        return target->m_game->call_event(event_type::check_card_taker{ target, card_taker_type::discards });
    }

    void equip_gary_looter::on_enable(card_ptr target_card, player_ptr player_end) {
        player_end->m_game->add_listener<event_type::check_card_taker>(target_card, [=](player_ptr e_target, card_taker_type type) -> card_ptr {
            if (type == card_taker_type::discards && e_target == player_end) {
                return target_card;
            }
            return nullptr;
        });
        player_end->m_game->add_listener<event_type::on_discard_pass>(target_card, [=](player_ptr player_begin, card_ptr discarded_card) {
            if (player_begin != player_end) {
                if (rn::none_of(player_begin->m_game->range_all_players(player_begin)
                    | rv::take_while([=](const_player_ptr current) { return current != player_end; })
                    | rv::filter(&player::alive),
                    get_gary_looter)
                ) {
                    player_end->m_game->add_log("LOG_DRAWN_CARD", player_end, discarded_card);
                    discarded_card->add_short_pause();
                    player_end->add_to_hand(discarded_card);
                }
            }
        });
    }
}