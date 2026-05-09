#include "posse.h"

#include "ruleset.h"

#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    void equip_posse::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_start>({ target_card, 6 }, [=](player_ptr origin) {
            if (origin == target) {
                if (is_tracked_player(target_card, target)) {
                    target->discard_card(target_card);
                    target->damage(target_card, nullptr, 1);
                } else {
                    for (player_ptr dest : target->m_game->range_other_players(target)) {
                        if (!dest->find_equipped_card(target_card)) {
                            target->disable_equip(target_card);
                            dest->equip_card(target_card);
                            break;
                        }
                    }
                }
            }
        });
    }
}