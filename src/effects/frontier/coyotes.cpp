#include "coyotes.h"

#include "effects/base/predraw_check.h"
#include "effects/base/pick.h"
#include "effects/base/resolve.h"

#include "cards/game_enums.h"

#include "game/game_table.h"
#include "game/prompts.h"

namespace banggame {

    struct request_coyotes : request_resolvable, interface_picking {
        using request_resolvable::request_resolvable;

        void on_update() override {
            if (!target->alive()) {
                pop_request();
            } else if (rn::none_of(target->m_hand, [&](card_ptr c) { return can_pick(c); })) {
                target->reveal_hand();
                on_resolve();
            } else {
                auto_resolve();
            }
        }

        prompt_string pick_prompt(card_ptr target_card) const override {
            return prompts::bot_check_discard_card(target, target_card);
        }

        bool can_pick(card_ptr target_card) const override {
            return target_card->pocket == pocket_type::player_hand && target_card->owner == target
                && !target->m_game->is_usage_disabled(target_card);
        }

        void on_pick(card_ptr target_card) override {
            pop_request();
            target->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, target, target_card);
            target->discard_used_card(target_card);
        }

        prompt_string resolve_prompt() const override {
            if (target->is_bot() && target->m_hp <= 2 && rn::any_of(target->m_hand, [&](card_ptr c) { return can_pick(c); })) {
                return "BOT_MUST_RESPOND_COYOTES";
            }
            return {};
        }

        void on_resolve() override {
            pop_request();
            target->damage(origin_card, nullptr, 1);
        }

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_COYOTES", origin_card};
            } else {
                return {"STATUS_COYOTES_OTHER", target, origin_card};
            }
        }
    };

    void equip_coyotes::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_predraw_check>(target_card, [=](player_ptr e_player, card_ptr e_card) {
            if (e_player == target && e_card == target_card) {
                target->m_game->queue_request<request_check>(target, target_card,
                    [](card_sign sign) {
                        return draw_check_result {
                            .lucky = !sign.is_spades(),
                            .defensive_redraw = true
                        };
                    }, [=](bool result) {
                        if (!result) {
                            target->m_game->queue_request<request_coyotes>(target_card, nullptr, target);
                        } else {
                            for (player_ptr dest : target->m_game->range_other_players(target)) {
                                if (!dest->find_equipped_card(target_card)) {
                                    target->disable_equip(target_card);
                                    dest->equip_card(target_card);
                                    break;
                                }
                            }
                        }
                    }
                );
            }
        });
    }
}