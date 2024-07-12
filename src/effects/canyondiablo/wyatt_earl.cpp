#include "wyatt_earl.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    void equip_wyatt_earl::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::apply_immunity_modifier>(target_card, [=](card *origin_card, player *e_origin, const player *e_target, effect_flags flags, std::vector<card *> &cards) {
            if (origin_card && e_target == target && flags.check(effect_flag::multi_target)) {
                cards.emplace_back(target_card);
            }
        });
    }
}