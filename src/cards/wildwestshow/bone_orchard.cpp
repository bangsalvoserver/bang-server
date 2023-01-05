#include "bone_orchard.h"

#include "game/game.h"

namespace banggame {

    void equip_bone_orchard::on_enable(card *target_card, player *origin) {
        origin->m_game->add_listener<event_type::verify_revivers>(target_card, [=](player *target) {
            if (!target->alive()) {
                target->m_game->flash_card(target_card);
                origin->m_game->add_log("LOG_REVIVE", target, target_card);

                target->remove_player_flags(player_flags::dead);
                target->set_hp(1);

                for (auto *c : target->m_characters) {
                    c->on_enable(target);
                }

                // TODO set random role from dead players
            }
        });
    }
}