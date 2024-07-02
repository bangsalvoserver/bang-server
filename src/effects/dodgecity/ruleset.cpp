#include "ruleset.h"

#include "game/game.h"

namespace banggame {

    void ruleset_dodgecity::on_apply(game *game) {
        game->add_listener<event_type::on_equip_card>({nullptr, 5}, [](player *origin, player *target, card *target_card, const effect_context &ctx) {
            if (target_card->is_green()) {
                target_card->set_inactive(true);
            }
        });

        game->add_listener<event_type::on_turn_end>({nullptr, 5}, [](player *origin, bool skipped) {
            for (card *target_card : origin->m_table) {
                target_card->set_inactive(false);
            }
        });
    }
}