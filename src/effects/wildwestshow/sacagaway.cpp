#include "sacagaway.h"

#include "cards/game_enums.h"

#include "game/game_table.h"

namespace banggame {
    
    void equip_sacagaway::on_enable(card_ptr target_card, player_ptr target) {
        for (player_ptr p : target->m_game->range_alive_players(target)) {
            for (card_ptr c : p->m_hand) {
                target->m_game->add_log("LOG_REVEALED_CARD", p, c);
                c->set_visibility(card_visibility::shown);
            }
        }
    }

    void equip_sacagaway::on_disable(card_ptr target_card, player_ptr target) {
        for (player_ptr p : target->m_game->range_alive_players(target)) {
            for (card_ptr c : p->m_hand) {
                c->set_visibility(card_visibility::show_owner, p);
            }
        }
    }
}