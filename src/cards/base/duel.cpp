#include "duel.h"

#include "game/game.h"

namespace banggame {

    struct request_duel : request_base, resolvable_request {
        request_duel(card *origin_card, player *origin, player *target, player *respond_to, effect_flags flags = {})
            : request_base(origin_card, origin, target, flags)
            , respond_to(respond_to) {}

        player *respond_to = nullptr;

        void on_update() override {
            if (!sent) {
                target->m_game->play_sound(target, "duel");
            }
            if (target->empty_hand()) {
                auto_respond();
            }
        }

        bool can_pick(card *target_card) const override {
            return target_card->pocket == pocket_type::player_hand && target_card->owner == target
                && target->is_bangcard(target_card)
                && !target->m_game->is_disabled(target_card);
        }

        void on_pick(card *target_card) override {
            auto lock = target->m_game->lock_updates(true);
            target->m_game->add_log("LOG_RESPONDED_WITH_CARD", target_card, target);
            target->discard_card(target_card);
            target->m_game->call_event<event_type::on_play_hand_card>(target, target_card);
            target->m_game->queue_request<request_duel>(origin_card, origin, respond_to, target);
        }

        void on_resolve() override {
            auto lock = target->m_game->lock_updates(true);
            target->damage(origin_card, origin, 1);
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_DUEL", origin_card};
            } else {
                return {"STATUS_DUEL_OTHER", target, origin_card};
            }
        }
    };

    void effect_duel::on_play(card *origin_card, player *origin, player *target, effect_flags flags) {
        target->m_game->add_log("LOG_PLAYED_CARD_ON", origin_card, origin, target);
        target->m_game->queue_request<request_duel>(origin_card, origin, target, origin, flags);
    }
}