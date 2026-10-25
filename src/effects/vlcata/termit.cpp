#include "termit.h"

#include <set>

#include "cards/game_events.h"
#include "effects/base/generalstore.h"

#include "game/game_table.h"

namespace banggame {

    void equip_termit::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::count_initial_cards>(target_card, [target](const_player_ptr origin) -> std::optional<int> {
            if (origin != target) return std::nullopt;

            auto &players = target->m_game->m_players;
            size_t n = players.size();
            size_t first_idx = n;
            for (size_t i = 0; i < n; ++i) {
                if (players[i] == target->m_game->m_first_player) {
                    first_idx = i;
                    break;
                }
            }
            if (first_idx == n || n < 2) return std::nullopt;

            size_t last_idx = (first_idx + n - 1) % n;
            size_t second_last_idx = (first_idx + n - 2) % n;
            if (players[last_idx] == target || players[second_last_idx] == target) {
                return target->m_hp + 1;
            }
            return std::nullopt;
        });

        target->m_game->add_listener<event_type::count_generalstore_cards>(target_card, [target](int &value) {
            if (target->alive()) {
                ++value;
            }
        });

        // Grant Termit's bonus pick right when his own regular pick comes up in the
        // Emporio sequence (priority 101 > the default 100 used by the other still-queued
        // per-player picks), instead of firing it early at the moment General Store is
        // played, which let him pick out of turn order.
        auto bonused_stores = std::make_shared<std::set<card_ptr>>();
        target->m_game->add_listener<event_type::on_generalstore_pick>(target_card,
            [target, target_card, bonused_stores](player_ptr origin, card_ptr origin_card, player_ptr pick_target, card_ptr picked_card) {
                if (pick_target == target && target->alive() && origin_card->name == "GENERAL_STORE"
                    && !bonused_stores->contains(origin_card)) {
                    bonused_stores->insert(origin_card);
                    target_card->flash_card();
                    target->m_game->queue_request<request_generalstore>(origin_card, origin, target, effect_flags{}, 101);
                }
            });

        target->m_game->add_listener<event_type::apply_maxcards_modifier>(target_card, [target](const_player_ptr origin, int &value) {
            if (origin == target && target->m_hp < 3) {
                ++value;
            }
        });
    }

}
