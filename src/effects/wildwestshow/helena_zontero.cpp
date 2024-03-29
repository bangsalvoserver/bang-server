#include "helena_zontero.h"

#include "game/game.h"

#include "cards/game_enums.h"

namespace banggame {

    void equip_helena_zontero::on_enable(card *target_card, player *origin) {
        origin->m_game->draw_check_then(nullptr, target_card, &card_sign::is_red, [=](bool condition) {
            if (condition) {
                auto alive_players = origin->m_game->m_players
                    | rv::filter([](player *p) {
                        return p->alive() && p->m_role != player_role::sheriff;
                    });
                
                auto roles = alive_players | rv::transform(&player::m_role) | rn::to_vector;
                rn::shuffle(roles, origin->m_game->rng);
                
                for (player *p : alive_players) {
                    p->set_role(player_role::unknown, false);
                    p->remove_player_flags(player_flags::role_revealed);
                }

                for (auto [p, role] : rv::zip(alive_players, roles)) {
                    p->set_role(role, false);
                }
            }
        });
    }
}