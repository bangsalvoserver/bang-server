#include "frankie_canton.h"

#include "game/game.h"
#include "game/prompts.h"

namespace banggame {

    game_string effect_frankie_canton::get_error(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        if (target_card == origin->first_character()) {
            return "ERROR_TARGET_PLAYING_CARD";
        }
        if (target_card->num_cubes == 0) {
            return {"ERROR_NOT_ENOUGH_CUBES_ON", target_card};
        }
        return {};
    }

    game_string effect_frankie_canton::on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        MAYBE_RETURN(prompts::bot_check_target_enemy(origin, target_card->owner));
        MAYBE_RETURN(prompts::prompt_target_self(origin_card, origin, target_card->owner));
        return {};
    }

    void effect_frankie_canton::on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        origin->m_game->add_log("LOG_PLAYED_CARD_ON", origin_card, origin, target_card);
        target_card->move_cubes(origin->first_character(), 1);
    }
}