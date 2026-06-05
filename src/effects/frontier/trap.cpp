#include "trap.h"

#include "effects/base/equip.h"
#include "effects/base/draw_check.h"
#include "effects/base/pick.h"

#include "game/game_table.h"
#include "game/prompts.h"

namespace banggame {

    game_string equip_trap::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        MAYBE_RETURN(prompts::prompt_target_self(origin_card, origin, target));
        MAYBE_RETURN(prompts::bot_check_target_enemy(origin, target));
        return {};
    }

    void equip_trap::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::check_equip_card>(target_card, [=](player_ptr origin, card_ptr origin_card, player_ptr e_target, const effect_context &ctx) -> game_string {
            if (origin == target && origin_card->is_blue()) {
                return "ERROR_CANT_EQUIP_BLUE_CARDS";
            }
            return {};
        });
    }

    struct request_move_trap : request_picking_player {
        request_move_trap(card_ptr origin_card, player_ptr target)
            : request_picking_player(origin_card, nullptr, target) {}
        
        bool can_pick(player_ptr target_player) const override {
            return target_player != target
                && !target_player->find_equipped_card(origin_card);
        }

        void on_pick(player_ptr target_player) override {
            pop_request();

            target->m_game->add_log("LOG_MOVE_TRAP_ON", origin_card, target, target_player);
            target->disable_equip(origin_card);
            target_player->equip_card(origin_card);
        }

        prompt_string pick_prompt(player_ptr target_player) const override {
            return prompts::bot_check_target_enemy(target, target_player);
        }

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_MOVE_TRAP", origin_card};
            } else {
                return {"STATUS_MOVE_TRAP_OTHER", target, origin_card};
            }
        }
    };

    void effect_trap::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->queue_request<request_check>(origin, origin_card, std::not_fn(&card_sign::is_spades),
            [=](card_sign sign) {
                switch (sign.suit) {
                case card_suit::hearts:
                    origin->m_game->queue_request<request_move_trap>(origin_card, origin);
                    break;
                case card_suit::spades:
                    origin->discard_card(origin_card);
                    origin->damage(origin_card, nullptr, 1);
                    break;
                default:
                    // ignore
                    break;
                }
            }
        );
    }
}