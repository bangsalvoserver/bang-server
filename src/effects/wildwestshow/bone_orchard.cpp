#include "bone_orchard.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    void equip_bone_orchard::on_enable(card *target_card, player *origin) {
        origin->m_game->add_listener<event_type::check_revivers>({target_card, -1}, [=](player *target) {
            if (!target->alive()) {
                target->m_game->flash_card(target_card);
                target->m_game->add_log("LOG_REVIVE", target, target_card);

                auto dead_players = target->m_game->m_players | rv::remove_if(&player::alive);
                if (rn::distance(dead_players) > 1) {
                    auto roles = dead_players | rv::transform(&player::m_role) | rn::to_vector;
                    rn::shuffle(roles, origin->m_game->rng);
                    
                    for (player *p : dead_players) {
                        p->set_role(player_role::unknown, false);
                        p->remove_player_flags(player_flags::role_revealed);
                    }

                    for (auto [p, role] : rv::zip(dead_players, roles)) {
                        p->set_role(role, false);
                    }
                }

                target->remove_player_flags(player_flags::dead);
                target->set_hp(1);

                for (auto *c : target->m_characters) {
                    target->enable_equip(c);
                }
            }
        });
    }
}