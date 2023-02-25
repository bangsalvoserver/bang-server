#include "john_pain.h"

#include "game/game.h"

namespace banggame {

    static constexpr int equip_type_john_pain = 3;

    static card *get_john_pain(player *target) {
        return target->m_game->call_event<event_type::check_card_taker>(target, equip_type_john_pain, nullptr);
    }
    
    void equip_john_pain::on_enable(card *target_card, player *player_end) {
        player_end->m_game->add_listener<event_type::check_card_taker>(target_card, [=](player *e_target, int type, card* &value) {
            if (type == equip_type_john_pain && e_target == player_end && e_target->m_hand.size() < 6) {
                value = target_card;
            }
        });
        player_end->m_game->add_listener<event_type::on_draw_check_resolve>(target_card, [=](player *player_begin, card *drawn_card) {
            if (!player_begin) return;
            
            player_end->m_game->queue_action([=]{
                if (drawn_card->pocket != pocket_type::player_hand
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