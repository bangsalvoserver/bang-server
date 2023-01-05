#include "helena_zontero.h"

#include "game/game.h"

namespace banggame {

    void equip_helena_zontero::on_enable(card *target_card, player *origin) {
        origin->m_game->draw_check_then(nullptr, target_card, [](card_sign sign) {
            return sign.suit == card_suit::hearts || sign.suit == card_suit::diamonds;
        }, [=](bool condition) {
            if (condition) {
                auto alive_players = origin->m_game->m_players
                    | ranges::views::filter([](player *p) {
                        return p->alive() && p->m_role != player_role::sheriff;
                    });
                for (player *p : alive_players) {
                    if (origin->m_game->m_players.size() > 3) {
                        origin->m_game->add_update<game_update_type::player_show_role>(update_target::includes(p), p, player_role::unknown);
                    } else {
                        origin->m_game->add_update<game_update_type::player_show_role>(p, player_role::unknown);
                    }
                }

                auto roles = alive_players | ranges::views::transform(&player::m_role) | ranges::to<std::vector>;
                std::ranges::shuffle(roles, origin->m_game->rng);
                for (auto [p, role] : ranges::views::zip(alive_players, roles)) {
                    p->set_role(role);
                }
            }
        });
    }
}