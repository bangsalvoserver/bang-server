#include "evelyn_shebang.h"

#include "game/game.h"
#include "cards/base/bang.h"

namespace banggame {

    game_string effect_evelyn_shebang::get_error(card *origin_card, player *origin, player *target) {
        game_string out_error;
        origin->m_game->call_event(event_type::check_bang_target{ origin_card, origin, target, effect_flags{}, out_error });
        return out_error;
    }

    void effect_evelyn_shebang::on_play(card *origin_card, player *origin, player *target) {
        origin->m_game->add_listener<event_type::check_bang_target>(origin_card, [=](card *e_origin_card, player *e_origin, player *e_target, effect_flags flags, game_string &out_error) {
            if (e_origin_card == origin_card && e_origin == origin && e_target == target) {
                out_error = "ERROR_TARGET_NOT_UNIQUE";
            }
        });

        origin->m_game->add_listener<event_type::on_turn_end>(origin_card, [=](player *e_origin, bool skipped) {
            if (origin == e_origin) {
                origin->m_game->remove_listeners(origin_card);
            }
        });

        effect_bang().on_play(origin_card, origin, target);
    }
}