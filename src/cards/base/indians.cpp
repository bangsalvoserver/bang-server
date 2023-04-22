#include "indians.h"

#include "cards/game_enums.h"
#include "cards/filters.h"
#include "bang.h"

#include "game/game.h"

namespace banggame {

    struct request_indians : request_base, resolvable_request, respondable_with_bang {
        request_indians(card *origin_card, player *origin, player *target, effect_flags flags = {})
            : request_base(origin_card, origin, target, flags) {}

        void on_update() override {
            if (target->immune_to(origin_card, origin, flags)) {
                target->m_game->pop_request();
            } else {
                if (state == request_state::pending) {
                    target->m_game->play_sound(target, "indians");
                }
                if (target->empty_hand()) {
                    auto_respond();
                }
            }
        }

        bool can_pick(card *target_card) const override {
            return target_card->pocket == pocket_type::player_hand && target_card->owner == target && filters::is_bang_card(target, target_card);
        }

        void respond_with_bang() override {
            target->m_game->pop_request();
        }
        
        void on_pick(card *target_card) override {
            target->m_game->add_log("LOG_RESPONDED_WITH_CARD", target_card, target);
            target->discard_used_card(target_card);
            respond_with_bang();
        }

        void on_resolve() override {
            target->m_game->pop_request();
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