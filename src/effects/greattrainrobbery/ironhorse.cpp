#include "ironhorse.h"

#include "game/game_table.h"
#include "game/play_verify.h"

#include "effects/base/bang.h"

#include "cards/game_enums.h"

#include "ruleset.h"

namespace banggame {

    static prompt_string get_ironhorse_prompt(player_ptr origin, player_ptr target, int locomotive_count) {
        if (origin->is_bot()) {
            auto role = origin->get_base_role();
            if (!(role == player_role::outlaw || role == player_role::renegade && origin->m_game->num_alive() <= 2)
                && (target->m_hp <= locomotive_count && target->m_role == player_role::sheriff)
            ) {
                return {1, "BOT_DONT_KILL_SHERIFF"};
            }
        }
        return {};
    }

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

        origin->m_game->add_listener<event_type::get_locomotive_prompt>(origin_card, [](player_ptr target, int locomotive_count, prompt_string &out_prompt) {
            out_prompt = select_prompt_fallback_empty(target->m_game->range_alive_players(target) | rv::transform([&](player_ptr p){
                return get_ironhorse_prompt(target, p, locomotive_count);
            }));
        });
    }
}