#include "bandidos.h"

#include "game/game.h"

namespace banggame {

    struct request_bandidos : request_base, resolvable_request {
        request_bandidos(card *origin_card, player *origin, player *target, effect_flags flags = {})
            : request_base(origin_card, origin, target, flags | effect_flags::auto_respond) {}

        int num_cards = 2;

        bool auto_resolve() override {
            if (num_cards == 2) {
                target->m_game->play_sound(target, "bandidos");
            }
            return request_base::auto_resolve();
        }

        bool can_pick(card *target_card) const override {
            return target_card->pocket == pocket_type::player_hand && target_card->owner == target;
        }

        void on_pick(card *target_card) override {
            target->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, target, target_card);
            target->discard_card(target_card);
            if (--num_cards == 0 || target->m_hand.empty()) {
                target->m_game->pop_request();
            } else {
                flags &= ~effect_flags::escapable;
            }
            target->m_game->update_request();
        }

        void on_resolve() override {
            if (num_cards == 2) {
                target->m_game->pop_request();
                target->damage(origin_card, origin, 1);
                target->m_game->update_request();
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