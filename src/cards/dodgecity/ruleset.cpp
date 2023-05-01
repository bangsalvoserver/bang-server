#include "ruleset.h"

#include "game/game.h"

namespace banggame {

    void ruleset_dodgecity::on_apply(game *game) {
        game->add_listener<event_type::on_equip_card>({nullptr, 5}, [](player *origin, player *target, card *origin_card, const effect_context &ctx) {
            if (origin_card->is_green() && !origin_card->inactive) {
                origin_card->inactive = true;
                origin->m_game->add_update<game_update_type::tap_card>(origin_card, true);
            }
        });
    }
}