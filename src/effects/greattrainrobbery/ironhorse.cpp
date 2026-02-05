#include "ironhorse.h"

#include "game/game_table.h"

#include "effects/base/bang.h"

#include "cards/game_enums.h"

#include "ruleset.h"

namespace banggame {

    void equip_ironhorse::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::on_locomotive_effect>(origin_card, [=](player_ptr target, shared_locomotive_context ctx) {
            origin->m_game->queue_action([=]{
                for (player_ptr p : target->m_game->range_alive_players(target)) {
                    if (p != ctx->skipped_player) {
                        origin->m_game->queue_request<request_bang>(origin_card, nullptr, p, effect_flag::target_players);
                    }
                }
            });
        });

        origin->m_game->add_listener<event_type::get_locomotive_prompt>(origin_card, [](player_ptr target, int locomotive_count) -> game_string {
            if (target->is_bot()) {
                player_ptr sheriff = target->m_game->m_first_player;
                auto role = target->get_base_role();
                if (!(role == player_role::outlaw || role == player_role::renegade && target->m_game->num_alive() <= 2)
                    && (sheriff->m_hp <= locomotive_count && sheriff->is_sheriff())
                ) {
                    return "BOT_DONT_KILL_SHERIFF";
                }
            }
            return {};
        });
    }
}