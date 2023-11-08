#include "mail_car.h"

#include "game/game.h"

#include "cards/game_enums.h"

namespace banggame {

    struct request_mail_car : request_auto_select {
        request_mail_car(card *origin_card, player *origin)
            : request_auto_select(origin_card, nullptr, origin) {}
        
        void on_update() override {
            if (state == request_state::pending) {
                for (int i=0; i<3; ++i) {
                    target->m_game->move_card(target->m_game->top_of_deck(), pocket_type::selection, target);
                }
            }
        }

        game_string status_text(player *owner) const override {
            if (owner == target) {
                return {"STATUS_MAIL_CAR", origin_card};
            } else {
                return {"STATUS_MAIL_CAR_OTHER", target, origin_card};
            }
        }
    };

    void effect_mail_car::on_play(card *origin_card, player *origin) {
        origin->m_game->queue_request<request_mail_car>(origin_card, origin);
    }

    game_string handler_mail_car::get_error(card *origin_card, player *origin, card *target_card, player *target_player) {
        if (origin->m_game->top_request<request_mail_car>(origin) == nullptr) {
            return "ERROR_INVALID_RESPONSE";
        }
        return {};
    }

    void handler_mail_car::on_play(card *origin_card, player *origin, card *target_card, player *target_player) {
        origin->m_game->pop_request();

        if (!origin->m_game->check_flags(game_flags::hands_shown)) {
            origin->m_game->add_log(update_target::includes(origin, target_player), "LOG_GIFTED_CARD", origin, target_player, target_card);
            origin->m_game->add_log(update_target::excludes(origin, target_player), "LOG_GIFTED_A_CARD", origin, target_player);
        } else {
            origin->m_game->add_log("LOG_GIFTED_CARD", origin, target_player, target_card);
        }
        target_player->add_to_hand(target_card);

        while (!origin->m_game->m_selection.empty()) {
            card *c = origin->m_game->m_selection.front();
            if (!origin->m_game->check_flags(game_flags::hands_shown)) {
                origin->m_game->add_log(update_target::includes(origin), "LOG_DRAWN_CARD", origin, c);
                origin->m_game->add_log(update_target::excludes(origin), "LOG_DRAWN_CARDS", origin, 1);
            } else {
                origin->m_game->add_log("LOG_DRAWN_CARD", origin, c);
            }
            origin->add_to_hand(c);
        }
    }
}