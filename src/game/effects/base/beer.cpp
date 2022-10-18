#include "beer.h"

#include "../../game.h"

namespace banggame {

    static bool can_use_beer(game *game) {
        if (game->m_players.size() <= 2) {
            return game->m_options.can_play_beer_in_duel;
        } else {
            return game->num_alive() > 2;
        }
    }
    
    game_string effect_beer::on_prompt(card *origin_card, player *origin, player *target) {
        if (!can_use_beer(target->m_game) || (target->m_hp == target->m_max_hp)) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        }
        return {};
    }

    void effect_beer::on_play(card *origin_card, player *origin, player *target) {
        if (can_use_beer(target->m_game)) {
            target->heal(target->m_game->call_event<event_type::apply_beer_modifier>(target, 1));
        }
        target->m_game->call_event<event_type::on_play_beer>(target);
    }
}