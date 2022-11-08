#include "gary_looter.h"

#include "game/game.h"

namespace banggame {

    static bool is_gary_looter(player &target) {
        return target.m_game->call_event<event_type::verify_card_taker>(&target, equip_type::gary_looter, false);
    }

    void effect_gary_looter::on_enable(card *target_card, player *player_end) {
        player_end->m_game->add_listener<event_type::verify_card_taker>(target_card, [=](player *e_target, equip_type type, bool &value) {
            if (type == equip_type::gary_looter && e_target == player_end) {
                value = true;
            }
        });
        player_end->m_game->add_listener<event_type::on_discard_pass>(target_card, [=](player *player_begin, card *discarded_card) {
            if (player_begin != player_end && std::none_of(player_iterator(player_begin), player_iterator(player_end), is_gary_looter)) {
                player_end->m_game->add_log("LOG_DRAWN_CARD", player_end, discarded_card);
                player_end->m_game->move_card(discarded_card, pocket_type::player_hand, player_end, show_card_flags::short_pause);
            }
        });
    }
}