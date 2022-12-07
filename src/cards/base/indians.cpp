#include "indians.h"

#include "game/game.h"

namespace banggame {

    struct request_indians : request_base, resolvable_request {
        request_indians(card *origin_card, player *origin, player *target, effect_flags flags = {})
            : request_base(origin_card, origin, target, flags) {}

        void on_update() override {
            target->m_game->play_sound(target, "indians");
        }

        bool auto_resolve() override {
            return target->empty_hand() && auto_respond();
        }

        bool can_pick(card *target_card) const override {
            return target_card->pocket == pocket_type::player_hand && target_card->owner == target && target->is_bangcard(target_card);
        }

        void on_pick(card *target_card) override {
            auto lock = target->m_game->lock_updates(true);
            target->m_game->add_log("LOG_RESPONDED_WITH_CARD", target_card, target);
            target->discard_card(target_card);
            target->m_game->call_event<event_type::on_play_hand_card>(target, target_card);
        }

        void on_resolve() override {
            auto lock = target->m_game->lock_updates(true);
            target->damage(origin_card, origin, 1);
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_INDIANS", origin_card};
            } else {
                return {"STATUS_INDIANS_OTHER", target, origin_card};
            }
        }
    };
    
    void effect_indians::on_play(card *origin_card, player *origin, player *target, effect_flags flags) {
        if (!bool(flags & effect_flags::multi_target)) {
            target->m_game->add_log("LOG_PLAYED_CARD_ON", origin_card, origin, target);
        }
        target->m_game->queue_request<request_indians>(origin_card, origin, target, flags);
    }
}