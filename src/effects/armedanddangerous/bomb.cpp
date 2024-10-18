#include "bomb.h"

#include "cards/game_enums.h"

#include "game/game.h"
#include "game/prompts.h"

#include "ruleset.h"

#include "effects/base/predraw_check.h"

namespace banggame {
    
    struct request_move_bomb : request_picking_player {
        request_move_bomb(card_ptr origin_card, player_ptr target)
            : request_picking_player(origin_card, nullptr, target, {}, 110) {}
        
        bool can_pick(const_player_ptr target_player) const override {
            if (target_player != target) {
                return !target_player->find_equipped_card(origin_card);
            }
            return true;
        }

        void on_pick(player_ptr target_player) override {
            target->m_game->pop_request();
            if (target != target_player) {
                target->m_game->add_log("LOG_MOVE_BOMB_ON", origin_card, target, target_player);
                target->disable_equip(origin_card);
                target_player->equip_card(origin_card);
            }
        }

        prompt_string pick_prompt(player_ptr target_player) const override {
            if (target == target_player) {
                return {"PROMPT_MOVE_BOMB_TO_SELF", origin_card};
            } else {
                return {};
            }
        }

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_MOVE_BOMB", origin_card};
            } else {
                return {"STATUS_MOVE_BOMB_OTHER", target, origin_card};
            }
        }
    };

    game_string equip_bomb::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        MAYBE_RETURN(prompts::bot_check_target_enemy(origin, target));
        MAYBE_RETURN(prompts::prompt_target_self(origin_card, origin, target));
        return {};
    }

    void equip_bomb::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_discard_orange_card>(target_card, [=](player_ptr e_target, card_ptr e_card) {
            if (e_target == target && e_card == target_card && !target->immune_to(target_card, nullptr, {})) {
                target->m_game->queue_action([=]{
                    target->m_game->add_log("LOG_CARD_EXPLODES", target_card);
                    target->m_game->play_sound("dynamite");
                    target->damage(target_card, nullptr, 2);
                }, 1);
            }
        });
        
        target->m_game->add_listener<event_type::on_predraw_check>(target_card, [=](player_ptr p, card_ptr e_card) {
            if (p == target && e_card == target_card) {
                target->m_game->queue_request<request_check>(target, target_card, &card_sign::is_red, [=](bool result) {
                    if (result) {
                        target->m_game->queue_request<request_move_bomb>(target_card, target);
                    } else {
                        target_card->move_cubes(nullptr, 2);
                    }
                });
            }
        });
    }

}