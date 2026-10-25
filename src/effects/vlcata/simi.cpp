#include "simi.h"

#include "cards/game_enums.h"
#include "effects/base/damage.h"

#include "game/game_table.h"
#include "game/game_options.h"
#include "game/request_timer.h"

#include "utils/range_utils.h"

namespace banggame {

    struct request_simi_reshuffle : request_base, request_timer {
        request_simi_reshuffle(card_ptr origin_card, player_ptr origin)
            : request_base(origin_card, origin, nullptr) {}

        void on_update() override {
            set_duration(origin->m_game->m_options.auto_resolve_timer);
        }

        void on_finished() override {
            pop_request();

            // No sheriff role exists in 3-player games, so the ability doesn't apply
            if (origin->m_game->m_players.size() <= 3) return;

            auto other_players = rv::filter(origin->m_game->m_players, [origin = origin](player_ptr p) {
                return p->alive() && p != origin;
            }) | rn::to<std::vector>();

            // Roles are drawn from ALL alive players (Simi included), with exactly one
            // "sheriff" entry removed from the pool -- whoever actually held it, Simi or
            // not -- so the other players are reshuffled among the remaining non-sheriff
            // roles and Simi becoming sheriff never produces a second sheriff.
            auto all_alive = rv::filter(origin->m_game->m_players, &player::alive) | rn::to<std::vector>();
            auto roles = all_alive | rv::transform(&player::get_base_role) | rn::to<std::vector>();
            if (auto it = rn::find(roles, player_role::sheriff); it != roles.end()) {
                roles.erase(it);
            }
            rn::shuffle(roles, origin->m_game->rng);

            origin->hide_role();
            origin->set_role(player_role::sheriff);

            for (auto [p, role] : rv::zip(other_players, roles)) {
                p->hide_role();
                p->set_role(role);
            }

            int old_max_hp = origin->m_max_hp;
            origin->m_max_hp = origin->get_character_max_hp();
            if (origin->m_max_hp > old_max_hp) {
                int delta = origin->m_max_hp - old_max_hp;
                origin->heal(origin_card, nullptr, delta);
                origin->draw_card(delta, origin_card);
            }
        }

        game_string status_text(player_ptr owner) const override {
            return "STATUS_ROLES_SHUFFLED";
        }
    };

    void equip_simi_dynamite_master::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_hit>(target_card, [target, target_card](card_ptr origin_card, player_ptr origin, player_ptr hit_target, int damage, effect_flags flags) {
            if (hit_target == target && damage > 0 && origin_card && origin_card->name == "DYNAMITE") {
                target_card->flash_card();
                target->heal(target_card, nullptr, damage);
            }
        });

        target->m_game->queue_request<request_simi_reshuffle>(target_card, target);
    }

    bool effect_simi_take_dynamite::can_play(card_ptr origin_card, player_ptr origin) {
        return rn::any_of(origin->m_game->m_discards, [](card_ptr c) { return c->name == "DYNAMITE"; });
    }

    void effect_simi_take_dynamite::on_play(card_ptr origin_card, player_ptr origin) {
        auto it = rn::find_if(origin->m_game->m_discards, [](card_ptr c) { return c->name == "DYNAMITE"; });
        if (it != origin->m_game->m_discards.end()) {
            card_ptr dynamite = *it;
            origin->m_game->add_log("LOG_EQUIPPED_CARD", dynamite, origin);
            origin->equip_card(dynamite);
        }
    }

}
