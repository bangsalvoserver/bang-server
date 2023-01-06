#include "apache_kid.h"

#include "game/game.h"

namespace banggame {

    void equip_apache_kid::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::apply_immunity_modifier>(target_card, [=](card *origin_card, player *e_origin, const player *e_target, effect_flags flags, bool &value) {
            if (origin_card && e_origin != e_target && e_target == target && target->m_game->get_card_sign(origin_card).suit == card_suit::diamonds) {
                target->m_game->add_log("LOG_PLAYER_IMMUNE_TO_CARD", target, origin_card, target_card);
                target->m_game->flash_card(target_card);
                value = true;
            }
        });
    }
}