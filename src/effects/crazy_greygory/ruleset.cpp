#include "ruleset.h"

#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    void ruleset_crazy_greygory::on_apply(game_ptr game) {
        game->add_listener<event_type::on_assign_characters>(nullptr, [](player_ptr first_player, bool &handled) {
            handled = true;

            card_ptr target_card = get_single_element(first_player->m_game->m_characters
                | rv::filter([](card_ptr target_card) {
                    return rn::contains(target_card->expansion, GET_RULESET(crazy_greygory));
                }));

            for (player_ptr p : first_player->m_game->range_alive_players(first_player)) {
                p->set_character(first_player->m_game->add_card(*target_card));
                p->set_hp(p->get_character_max_hp(), true);
            }
        });
    }
}