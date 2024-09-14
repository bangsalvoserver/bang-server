#include "eva_place.h"

#include "game/game.h"

#include "cards/game_enums.h"

#include "effects/base/pick.h"
#include "effects/base/resolve.h"

namespace banggame {

    struct request_eva_place : selection_picker, interface_resolvable {
        using selection_picker::selection_picker;

        card_ptr drawn_card;

        void on_update() override {
            drawn_card = target->m_game->top_of_deck();
            drawn_card->move_to(pocket_type::selection, target);
        }

        void on_pick(card_ptr) override {
            target->m_game->pop_request();

            target->m_game->add_log("LOG_DRAWN_CARD_FOR", target, drawn_card, origin_card);
            drawn_card->set_visibility(card_visibility::shown);
            drawn_card->add_short_pause();
            target->add_to_hand(drawn_card);

            if (drawn_card->sign.is_diamonds()) {
                target->draw_card(1, origin_card);
            }
        }

        int resolve_type() const override {
            return 1;
        }

        game_string resolve_prompt() const override {
            if (target->is_bot() && drawn_card->sign.is_diamonds()) {
                return "BOT_BAD_PLAY";
            }
            return {};
        }

        void on_resolve() override {
            target->m_game->pop_request();
            target->m_game->add_log("LOG_DRAWN_CARDS", target, 1);
            target->add_to_hand(drawn_card);
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_EVA_PLACE", origin_card};
            } else {
                return {"STATUS_EVA_PLACE_OTHER", target, origin_card};
            }
        }
    };

    void effect_eva_place::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->queue_request<request_eva_place>(origin_card, nullptr, origin);
    }
}