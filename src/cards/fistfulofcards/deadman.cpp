#include "deadman.h"

#include "game/game.h"

namespace banggame {
    
    void equip_deadman::on_enable(card *target_card, player *origin) {
        origin->m_game->add_listener<event_type::check_revivers>(target_card, [=](player *target) {
            if (!target->alive() && target == origin->m_game->m_first_dead) {
                target->m_game->flash_card(target_card);
                origin->m_game->add_log("LOG_REVIVE", target, target_card);

                target->remove_player_flags(player_flags::dead);
                target->set_hp(2);
                target->draw_card(2);

                for (auto *c : target->m_characters) {
                    target->enable_equip(c);
                }
            }
        });
    }
}