#include "bandolier.h"

#include "game/game.h"

namespace banggame {

    void effect_bandolier::add_context(card *origin_card, player *origin, effect_context &ctx) {
        ctx.disable_banglimit = true;
    }

    game_string effect_bandolier::on_prompt(card *origin_card, player *origin) {
        if (origin->get_bangs_played() == 0) {
            return {"PROMPT_NO_BANGS_PLAYED", origin_card};
        } else {
            return {};
        }
    }

    void effect_bandolier::on_play(card *origin_card, player *origin) {
        event_card_key key{origin_card, 4};
        origin->m_game->add_listener<event_type::count_bangs_played>(key, [=](player *p, int &value) {
            if (origin == p) {
                --value;
            }
        });
        origin->m_game->add_listener<event_type::on_turn_end>(key, [=](player *p, bool skipped) {
            if (origin == p) {
                origin->m_game->remove_listeners(key);
            }
        });
    }
}