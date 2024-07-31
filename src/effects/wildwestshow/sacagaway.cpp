#include "sacagaway.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {
    
    void equip_sacagaway::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_game_flags(game_flag::hands_shown);

        for (player_ptr p : range_alive_players(target)) {
            for (card_ptr c : p->m_hand) {
                c->set_visibility(card_visibility::shown);
            }
        }
    }

    void equip_sacagaway::on_disable(card_ptr target_card, player_ptr target) {
        for (player_ptr p : range_alive_players(target)) {
            for (card_ptr c : p->m_hand) {
                c->set_visibility(card_visibility::show_owner, p);
            }
        }

        target->m_game->remove_game_flags(game_flag::hands_shown);
    }
}