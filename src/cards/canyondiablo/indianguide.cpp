#include "indianguide.h"

#include "game/game.h"

namespace banggame {

    void effect_indianguide::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::apply_immunity_modifier>(target_card, [=](card *origin_card, player *e_origin, const player *e_target, effect_flags flags, bool &value) {
            if (e_target == target && origin_card->has_tag(tag_type::indians)) {
                target->m_game->add_log("LOG_PLAYER_IMMUNE_TO_CARD", target, origin_card, target_card);
                target->m_game->flash_card(target_card);
                value = true;
            }
        });
    }
}