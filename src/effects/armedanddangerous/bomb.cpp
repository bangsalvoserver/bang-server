#include "bomb.h"

#include "cards/game_enums.h"

#include "game/game.h"

#include "ruleset.h"

#include "effects/base/predraw_check.h"

namespace banggame {
    
    struct request_move_bomb : request_base {
        request_move_bomb(card_ptr origin_card, player_ptr target)
            : request_base(origin_card, nullptr, target, {}, 110) {}

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_MOVE_BOMB", origin_card};
            } else {
                return {"STATUS_MOVE_BOMB_OTHER", target, origin_card};
            }
        }
    };

    game_string effect_move_bomb::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        if (origin == target) {
            return {"PROMPT_MOVE_BOMB_TO_SELF", origin_card};
        } else {
            return {};
        }
    }

    game_string effect_move_bomb::get_error(card_ptr origin_card, player_ptr origin, player_ptr target) {
        if (!origin->m_game->top_request<request_move_bomb>(origin)) {
            return "ERROR_INVALID_RESPONSE";
        }
        if (target != origin) {
            if (auto c = target->find_equipped_card(origin_card)) {
                return {"ERROR_DUPLICATED_CARD", c};
            }
        }
        return {};
    }

    void effect_move_bomb::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        origin->m_game->pop_request();
        if (target != origin) {
            origin->m_game->add_log("LOG_MOVE_BOMB_ON", origin_card, origin, target);
            origin->disable_equip(origin_card);
            target->equip_card(origin_card);
        }
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