#include "bone_orchard.h"

#include "game/game.h"

namespace banggame {

    void equip_bone_orchard::on_enable(card *target_card, player *origin) {
        origin->m_game->add_listener<event_type::verify_revivers>(target_card, [=](player *target) {
            if (!target->alive()) {
                target->m_game->flash_card(target_card);
                target->m_game->add_log("LOG_REVIVE", target, target_card);

                auto dead_players = target->m_game->m_players | ranges::views::remove_if(&player::alive);
                if (ranges::distance(dead_players) > 1) {
                    for (player *p : dead_players) {
                        if (p->remove_player_flags(player_flags::role_revealed)) {
                            target->m_game->add_update<game_update_type::player_show_role>(p, player_role::unknown);
                        }
                    }

                    auto roles = dead_players | ranges::views::transform(&player::m_role) | ranges::to<std::vector>;
                    std::ranges::shuffle(roles, origin->m_game->rng);
                    for (auto [p, role] : ranges::views::zip(dead_players, roles)) {
                        p->set_role(role);
                    }
                }

                target->remove_player_flags(player_flags::dead);
                target->set_hp(1);

                for (auto *c : target->m_characters) {
                    c->on_enable(target);
                }
            }
        });
    }
}