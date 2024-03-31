#include "wanted.h"

#include "game/game.h"

#include "effects/base/deathsave.h"

namespace banggame {

    game_string equip_wanted::on_prompt(card *origin_card, player *origin, player *target) {
        MAYBE_RETURN(prompt_target_self::on_prompt(origin_card, origin, target));
        if (target->m_role == player_role::sheriff) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        } else {
            return {};
        }
    }

    void equip_wanted::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::on_player_death>({target_card, 3}, [p, target_card](player *origin, player *target) {
            if (origin && origin->alive() && p == target && origin != target) {
                origin->m_game->flash_card(target_card);
                origin->draw_card(2, target_card);
                origin->add_gold(1);
            }
        });
    }
}