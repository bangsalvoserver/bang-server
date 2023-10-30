#include "ruleset.h"
#include "add_cube.h"

#include "game/game.h"

namespace banggame {

    void ruleset_armedanddangerous::on_apply(game *game) {
        game->add_listener<event_type::on_game_setup>({nullptr, 4}, [](player *origin){
            origin->m_game->add_update<game_update_type::add_cubes>(origin->m_game->num_cubes = 32);
        });

        game->add_listener<event_type::check_play_card>(nullptr, [](player *origin, card *origin_card, const effect_context &ctx, game_string &out_error) {
            if (origin_card->pocket == pocket_type::player_hand && origin_card->is_orange() && origin->m_game->num_cubes < 3) {
                out_error = "ERROR_NOT_ENOUGH_CUBES";
            }
        });

        game->add_listener<event_type::on_equip_card>(nullptr, [](player *origin, player *target, card *origin_card, const effect_context &ctx) {
            if (origin_card->is_blue()) {
                effect_add_cube{1}.on_play(origin_card, origin);
            } else if (origin_card->is_orange()) {
                origin->m_game->add_cubes(origin_card, 3);
            }
        });

        game->add_listener<event_type::post_discard_pass>(nullptr, [](player *target, int ndiscarded) {
            effect_add_cube{ndiscarded}.on_play(nullptr, target);
        });
    }
}