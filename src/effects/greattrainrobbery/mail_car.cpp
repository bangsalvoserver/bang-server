#include "mail_car.h"

#include "effects/base/prompts.h"

#include "game/game.h"

#include "cards/game_enums.h"

namespace banggame {

    struct request_mail_car : request_base {
        request_mail_car(card_ptr origin_card, player_ptr origin)
            : request_base(origin_card, nullptr, origin) {}
        
        void on_update() override {
            if (!live) {
                for (int i=0; i<3; ++i) {
                    target->m_game->top_of_deck()->move_to(pocket_type::selection, target);
                }
            }
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_MAIL_CAR", origin_card};
            } else {
                return {"STATUS_MAIL_CAR_OTHER", target, origin_card};
            }
        }
    };

    void effect_mail_car::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->queue_request<request_mail_car>(origin_card, origin);
    }

    bool effect_mail_car_response::can_play(card_ptr origin_card, player_ptr origin) {
        return origin->m_game->top_request<request_mail_car>(origin) != nullptr;
    }

    game_string handler_mail_car::on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target_card, player_ptr target_player) {
        MAYBE_RETURN(bot_check_target_friend(origin, target_player));
        return {};
    }

    void handler_mail_car::on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card, player_ptr target_player) {
        origin->m_game->pop_request();

        if (!origin->m_game->check_flags(game_flag::hands_shown)) {
            origin->m_game->add_log(update_target::includes(origin, target_player), "LOG_GIFTED_CARD", origin, target_player, target_card);
            origin->m_game->add_log(update_target::excludes(origin, target_player), "LOG_GIFTED_A_CARD", origin, target_player);
        } else {
            origin->m_game->add_log("LOG_GIFTED_CARD", origin, target_player, target_card);
        }
        target_player->add_to_hand(target_card);

        while (!origin->m_game->m_selection.empty()) {
            card_ptr c = origin->m_game->m_selection.front();
            if (!origin->m_game->check_flags(game_flag::hands_shown)) {
                origin->m_game->add_log(update_target::includes(origin), "LOG_DRAWN_CARD", origin, c);
                origin->m_game->add_log(update_target::excludes(origin), "LOG_DRAWN_CARDS", origin, 1);
            } else {
                origin->m_game->add_log("LOG_DRAWN_CARD", origin, c);
            }
            origin->add_to_hand(c);
        }
    }
}