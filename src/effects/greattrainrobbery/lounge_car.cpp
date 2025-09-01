#include "lounge_car.h"

#include "ruleset.h"

#include "cards/filter_enums.h"

#include "game/game_table.h"
#include "game/filters.h"
#include "game/prompts.h"

namespace banggame {

    struct request_lounge_car : request_base, interface_target_set_players {
        request_lounge_car(card_ptr origin_card, player_ptr origin)
            : request_base(origin_card, nullptr, origin) {}
        
        void on_update() override {
            if (update_count == 0) {
                for (int i=0; i<2 && !target->m_game->m_train_deck.empty(); ++i) {
                    target->m_game->m_train_deck.back()->move_to(pocket_type::selection, target);
                }
            }
            if (target->m_game->m_selection.empty()) {
                target->m_game->pop_request();
            }
        }

        void on_resolve(card_ptr target_card, player_ptr target_player) {
            target->m_game->add_log("LOG_EQUIPPED_CARD_TO", target_card, target, target_player);
            target_player->equip_card(target_card);

            while (!target->m_game->m_selection.empty()) {
                card_ptr c = target->m_game->m_selection.front();
                target->m_game->add_log("LOG_EQUIPPED_CARD", c, target);
                target->equip_card(c);
            }
        }

        bool in_target_set(const_player_ptr target_player) const override {
            return rn::any_of(target->m_game->m_selection, [&](card_ptr target_card) {
                return !check_player_filter(target_card, target, target_card->equip_target, target_player);
            });
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_LOUNGE_CAR", origin_card};
            } else {
                return {"STATUS_LOUNGE_CAR_OTHER", target, origin_card};
            }
        }
    };

    game_string effect_lounge_car::on_prompt(card_ptr origin_card, player_ptr origin) {
        if (origin->m_game->m_train_deck.empty()) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        }
        return {};
    }

    void effect_lounge_car::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->queue_request<request_lounge_car>(origin_card, origin);
    }

    bool effect_lounge_car_response::can_play(card_ptr origin_card, player_ptr origin) {
        return origin->m_game->top_request<request_lounge_car>(target_is{origin}) != nullptr;
    }

    game_string handler_lounge_car::get_error(card_ptr origin_card, player_ptr origin, card_ptr target_card, player_ptr target_player) {
        for (card_ptr selection_card : origin->m_game->m_selection) {
            MAYBE_RETURN(check_player_filter(selection_card, origin, selection_card->equip_target,
                selection_card == target_card ? target_player : origin));
        }
        return {};
    }

    game_string handler_lounge_car::on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target_card, player_ptr target_player) {
        if (target_card->has_tag(tag_type::penalty)) {
            MAYBE_RETURN(prompts::bot_check_target_enemy(origin, target_player));
        } else {
            MAYBE_RETURN(prompts::bot_check_target_friend(origin, target_player));
        }
        return {};
    }

    void handler_lounge_car::on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card, player_ptr target_player) {
        origin->m_game->top_request<request_lounge_car>()->on_resolve(target_card, target_player);
    }
}