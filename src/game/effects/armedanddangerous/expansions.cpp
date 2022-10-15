#include "expansions.h"

#include "../../game.h"

namespace banggame {
    void expansion_armedanddangerous::on_apply(game *game) {
        game->add_listener<event_type::on_game_setup>(nullptr, [=]{
            game->add_update<game_update_type::add_cubes>(game->num_cubes = 32);
        });

        game->add_listener<event_type::on_equip_card>(nullptr, [](player *origin, player *target, card *origin_card) {
            if (origin_card->color == card_color_type::blue) {
                origin->queue_request_add_cube(origin_card);
            } else if (origin_card->color == card_color_type::orange) {
                origin->add_cubes(origin_card, 3);
            }
        });

        game->add_listener<event_type::post_discard_pass>(nullptr, [](player *target, int ndiscarded) {
            target->queue_request_add_cube(nullptr, ndiscarded);
        });
    }
}