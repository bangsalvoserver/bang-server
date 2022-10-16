#include "ruleset.h"
#include "add_cube.h"

#include "../../game.h"

namespace banggame {

    void ruleset_armedanddangerous::on_apply(game *game) {
        game->add_listener<event_type::on_game_setup>(nullptr, [=]{
            game->add_update<game_update_type::add_cubes>(game->num_cubes = 32);
        });

        game->add_listener<event_type::on_equip_card>(nullptr, [](player *origin, player *target, card *origin_card) {
            if (origin_card->color == card_color_type::blue) {
                effect_add_cube{1}.on_play(origin_card, origin);
            } else if (origin_card->color == card_color_type::orange) {
                origin->add_cubes(origin_card, 3);
            }
        });

        game->add_listener<event_type::post_discard_pass>(nullptr, [](player *target, int ndiscarded) {
            effect_add_cube{ndiscarded}.on_play(nullptr, target);
        });
    }
}