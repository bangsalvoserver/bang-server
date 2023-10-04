#include "evelyn_shebang.h"

#include "game/game.h"
#include "cards/base/bang.h"

namespace banggame {

    game_string effect_evelyn_shebang::get_error(card *origin_card, player *origin, player *target) {
        return origin->m_game->call_event<event_type::check_card_target>(origin_card, origin, target, effect_flags{}, game_string{});
    }

    void effect_evelyn_shebang::on_play(card *origin_card, player *origin, player *target) {
        origin->m_game->add_listener<event_type::check_card_target>(origin_card, [=](card *e_origin_card, player *e_origin, player *e_target, effect_flags flags, game_string &out_error) {
            if (e_origin_card == origin_card && e_origin == origin && e_target == target) {
                out_error = "ERROR_TARGET_NOT_UNIQUE";
            }
        });

        origin->m_game->add_listener<event_type::on_turn_end>(origin_card, [=](player *e_origin, bool skipped) {
            if (origin == e_origin) {
                origin->m_game->remove_listeners(origin_card);
            }
        });

        ++origin->m_num_drawn_cards;

        effect_bang().on_play(origin_card, origin, target);
    }
}