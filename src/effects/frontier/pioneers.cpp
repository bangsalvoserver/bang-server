#include "pioneers.h"

#include "ruleset.h"

#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    void equip_pioneers::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_start>({ target_card, 5 }, [=](player_ptr origin) {
            if (origin == target) {
                if (is_tracked_player(target_card, target)) {
                    target->discard_card(target_card);
                    int ncards = rn::count_if(target->m_game->m_players, [&](player_ptr p) { return p->alive() && p != target; });
                    target->draw_card(ncards, target_card);
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