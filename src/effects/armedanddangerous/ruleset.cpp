#include "ruleset.h"
#include "add_cube.h"

#include "game/game.h"

#include "effects/base/requests.h"

namespace banggame {

    void ruleset_armedanddangerous::on_apply(game *game) {
        game->add_listener<event_type::on_game_setup>({nullptr, 4}, [](player_ptr origin){
            origin->m_game->add_tokens(card_token_type::cube, 32);
        });

        game->add_listener<event_type::check_play_card>(nullptr, [](player_ptr origin, card_ptr origin_card, const effect_context &ctx, game_string &out_error) {
            if (origin_card->pocket == pocket_type::player_hand && origin_card->is_orange() && origin->m_game->num_cubes() < 3) {
                out_error = "ERROR_NOT_ENOUGH_CUBES";
            }
        });

        game->add_listener<event_type::on_equip_card>(nullptr, [](player_ptr origin, player_ptr target, card_ptr origin_card, const effect_context &ctx) {
            if (origin_card->is_blue()) {
                effect_add_cube{1}.on_play(origin_card, origin);
            } else if (origin_card->is_orange()) {
                origin_card->add_cubes(3);
            }
        });

        game->add_listener<event_type::post_discard_pass>(nullptr, [](player_ptr target, int ndiscarded) {
            effect_add_cube{ndiscarded}.on_play(nullptr, target);
        });
    }
}