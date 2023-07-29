#include "wyatt_earl.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    void equip_wyatt_earl::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::apply_immunity_modifier>(target_card, [=](card *origin_card, player *e_origin, const player *e_target, effect_flags flags, bool &value) {
            if (origin_card && e_target == target && bool(flags & effect_flags::multi_target)) {
                target->m_game->add_log("LOG_PLAYER_IMMUNE_TO_CARD", target, origin_card, target_card);
                target->m_game->flash_card(target_card);
                value = true;
            }
        });
    }
}