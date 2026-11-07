#include "steve_tengo.h"

#include "ruleset.h"

#include "cards/game_events.h"
#include "cards/game_enums.h"

#include "effects/base/damage.h"
#include "effects/base/pick.h"

#include "game/game_table.h"
#include "game/prompts.h"
#include "game/bot_suggestion.h"

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
            apply_pardner_token(origin_card, target, target_player);
        }

        int evaluate_pick(player_ptr target_player) const override {
            return bot_suggestion::evaluate_neutral(target, target_player);
        }

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_TRACK_PLAYER", origin_card};
            } else {
                return {"STATUS_TRACK_PLAYER_OTHER", target, origin_card};
            }
        }
    };

    void equip_steve_tengo::on_enable(card_ptr target_card, player_ptr target) const {
        target->m_game->add_listener<event_type::on_turn_start>({ target_card, -6 }, [=](player_ptr origin) {
            if (origin == target) {
                target->m_game->queue_request<request_track_player>(target_card, nullptr, target);
            }
        });

        target->m_game->add_listener<event_type::on_hit>({ target_card, 5 }, [=](card_ptr origin_card, player_ptr origin, player_ptr e_target, int damage, effect_flags flags) {
            if (origin == target && get_tracked_player(target_card) == e_target) {
                target_card->flash_card();
                origin->draw_card(1, target_card);
            }
        });
    }

    void equip_steve_tengo::on_disable(card_ptr target_card, player_ptr target) const {
        target->m_game->remove_listeners({ target_card, -6 });
        target->m_game->remove_listeners({ target_card, 5 });
    }

    void equip_steve_tengo_nodisable::on_disable(card_ptr target_card, player_ptr target) const {
        remove_pardner_token(target_card, target);
    }
}
