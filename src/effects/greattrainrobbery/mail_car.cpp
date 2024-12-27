#include "mail_car.h"

#include "game/game.h"
#include "game/prompts.h"

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

    void effect_mail_car_response::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->pop_request();
    }
}