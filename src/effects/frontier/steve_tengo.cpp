#include "steve_tengo.h"

#include "ruleset.h"

#include "cards/game_events.h"
#include "cards/game_enums.h"

#include "effects/base/damage.h"
#include "effects/base/pick.h"

#include "game/game_table.h"
#include "game/prompts.h"

namespace banggame {

    struct request_track_player : request_picking_player {
        using request_picking_player::request_picking_player;

        bool can_pick(player_ptr target_player) const override {
            return target != target_player;
        }

        void on_update() override {
            if (auto valid_targets = target->m_game->m_players | rv::filter([&](player_ptr p) { return in_target_set(p); })) {
                if (player_ptr p = get_single_element(valid_targets)) on_pick(p);
            } else {
                pop_request();
            }
        }

        void on_pick(player_ptr target_player) override {
            pop_request();
            if (!is_tracked_player(origin_card, target_player)) {
                remove_pardner_token(origin_card, target);
                apply_pardner_token(origin_card, target, target_player);
            }
        }

        prompt_string pick_prompt(player_ptr target_player) const override {
            MAYBE_RETURN(prompts::bot_check_target_enemy(target, target_player));
            return {};
        }

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_TRACK_PLAYER", origin_card};
            } else {
                return {"STATUS_TRACK_PLAYER_OTHER", target, origin_card};
            }
        }
    };

    void equip_steve_tengo::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_start>({ target_card, -6 }, [=](player_ptr origin) {
            if (origin == target) {
                target->m_game->queue_request<request_track_player>(target_card, nullptr, target);
            }
        });

        target->m_game->add_listener<event_type::on_hit>({ target_card, 5 }, [=](card_ptr origin_card, player_ptr origin, player_ptr e_target, int damage, effect_flags flags) {
            if (origin == target && is_tracked_player(target_card, e_target)) {
                target_card->flash_card();
                origin->draw_card(1, target_card);
            }
        });
    }

    void equip_steve_tengo::on_disable(card_ptr target_card, player_ptr target) {
        if (!target->check_player_flags(player_flag::keep_alive) && !target->alive()) {
            remove_pardner_token(target_card, target);
        }

        target->m_game->remove_listeners(target_card);
    }
}