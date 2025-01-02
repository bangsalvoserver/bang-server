#include "sid_ketchum.h"

#include "effects/base/max_usages.h"
#include "effects/base/pick.h"
#include "effects/base/bang.h"
#include "effects/base/heal.h"

#include "game/game.h"

namespace banggame {

    struct request_sid_ketchum_legend : request_picking_player, interface_resolvable {
        using request_picking_player::request_picking_player;

        int resolve_type() const override {
            return 1;
        }

        void on_resolve() override {
            target->m_game->pop_request();
        }

        void on_update() override {
            if (rn::none_of(target->m_game->m_players, [&](const_player_ptr p) { return can_pick(p); })) {
                target->m_game->pop_request();
            }
        }

        bool can_pick(const_player_ptr target_player) const override {
            if (target_player == target) return false;
            int range = target->get_range_mod() + target->get_weapon_range();
            return target->m_game->calc_distance(target, target_player) <= range;
        }

        prompt_string pick_prompt(player_ptr target_player) const override {
            return effect_bang{}.on_prompt(origin_card, target, target_player);
        }

        void on_pick(player_ptr target_player) override {
            target->m_game->pop_request();

            effect_max_usages{1}.on_play(origin_card, target);
            effect_bang{}.on_play(origin_card, target, target_player);
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_SID_KETCHUM_LEGEND", origin_card};
            } else {
                return {"STATUS_SID_KETCHUM_LEGEND_OTHER", origin_card, target};
            }
        }
    };

    void equip_sid_ketchum_legend::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_heal>(target_card, [=](player_ptr origin) {
            if (origin == target && target->m_game->m_playing == target && !effect_max_usages{1}.get_error(target_card, target)) {
                target->m_game->queue_request<request_sid_ketchum_legend>(target_card, target, target);
            }
        });
    }
}