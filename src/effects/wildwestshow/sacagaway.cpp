#include "sacagaway.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {
    
    void equip_sacagaway::on_enable(card *target_card, player *target) {
        target->m_game->add_game_flags(game_flags::hands_shown);

        for (player *p : range_all_players(target)) {
            for (card *c : p->m_hand) {
                c->set_visibility(card_visibility::shown);
            }
        }
    }

    void equip_sacagaway::on_disable(card *target_card, player *target) {
        for (player *p : range_all_players(target)) {
            for (card *c : p->m_hand) {
                c->set_visibility(card_visibility::show_owner, p);
            }
        }

        target->m_game->remove_game_flags(game_flags::hands_shown);
    }
}