#include "bandidos.h"

#include "game/game.h"
#include "cards/base/requests.h"

namespace banggame {

    struct request_bandidos : request_base, resolvable_request {
        request_bandidos(card *origin_card, player *origin, player *target, effect_flags flags = {})
            : request_base(origin_card, origin, target, flags) {}

        void on_update() override {
            if (target->immune_to(origin_card, origin, flags)) {
                target->m_game->pop_request();
            } else {
                if (state == request_state::pending) {
                    target->m_game->play_sound(target, "bandidos");
                }
                auto_respond();
            }
        }

        void on_resolve() override {
            target->m_game->pop_request();
            target->damage(origin_card, origin, 1);
        }
        
        bool can_pick(card *target_card) const override {
            return target_card->pocket == pocket_type::player_hand && target_card->owner == target;
        }

        void on_pick(card *target_card) override {
            target->m_game->pop_request();
            target->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, target, target_card);
            target->discard_used_card(target_card);
            if (!target->empty_hand()) {
                target->m_game->queue_request_front<request_discard>(origin_card, origin, target);
            }
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_BANDIDOS", origin_card};
            } else {
                return {"STATUS_BANDIDOS_OTHER", target, origin_card};
            }
        }
    };

    void effect_bandidos::on_play(card *origin_card, player *origin, player *target, effect_flags flags) {
        target->m_game->queue_request<request_bandidos>(origin_card, origin, target, flags);
    }
}