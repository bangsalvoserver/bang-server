#include "mail_car.h"

#include "game/game_table.h"
#include "game/prompts.h"

#include "cards/game_enums.h"

namespace banggame {

    struct request_mail_car : request_base {
        request_mail_car(card_ptr origin_card, player_ptr origin)
            : request_base(origin_card, nullptr, origin) {}
        
        void on_update() override {
            if (update_count == 0) {
                for (int i=0; i<3; ++i) {
                    target->m_game->top_of_deck()->move_to(pocket_type::selection, target);
                }
            } else {
                target->m_game->pop_request();
                while (!target->m_game->m_selection.empty()) {
                    card_ptr c = target->m_game->m_selection.front();
                    if (!target->m_game->check_flags(game_flag::hands_shown)) {
                        target->m_game->add_log(update_target::includes(target), "LOG_DRAWN_CARD", target, c);
                        target->m_game->add_log(update_target::excludes(target), "LOG_DRAWN_CARDS", target, 1);
                    } else {
                        target->m_game->add_log("LOG_DRAWN_CARD", target, c);
                    }
                    target->add_to_hand(c);
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
        return origin->m_game->top_request<request_mail_car>(target_is{origin}) != nullptr;
    }
}