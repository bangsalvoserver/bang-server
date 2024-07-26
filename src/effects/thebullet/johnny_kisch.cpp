#include "johnny_kisch.h"

#include "game/game.h"

namespace banggame {

    void equip_johnny_kisch::on_enable(card_ptr target_card, player_ptr p) {
        p->m_game->add_listener<event_type::on_equip_card>(target_card, [=](player_ptr origin, player_ptr target, card_ptr equipped_card, const effect_context &ctx) {
            if (p == origin) {
                for (player_ptr other : range_other_players(target)) {
                    if (card_ptr card = other->find_equipped_card(equipped_card)) {
                        target->m_game->add_log("LOG_DISCARDED_CARD_FOR", target_card, other, card);
                        other->discard_card(card);
                    }
                }
            }
        });
    }
}