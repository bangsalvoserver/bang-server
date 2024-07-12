#include "indians.h"

#include "bang.h"
#include "pick.h"

#include "cards/game_enums.h"

#include "game/game.h"
#include "game/filters.h"

namespace banggame {

    struct request_indians : request_resolvable, interface_picking, respondable_with_bang {
        request_indians(card *origin_card, player *origin, player *target, effect_flags flags = {})
            : request_resolvable(origin_card, origin, target, flags) {}

        void on_update() override {
            if (target->immune_to(origin_card, origin, flags)) {
                target->m_game->pop_request();
            } else {
                if (!live) {
                    target->play_sound("indians");
                }
                if (target->empty_hand()) {
                    auto_resolve();
                }
            }
        }

        game_string resolve_prompt() const override {
            if (target->is_bot() && target->m_hp <= 1 && rn::any_of(target->m_hand, [&](card *target_card) { return can_pick(target_card); })) {
                return "BOT_BAD_PLAY";
            }
            return {};
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
        if (!flags.check(effect_flag::skip_target_logs)) {
            target->m_game->add_log("LOG_PLAYED_CARD_ON", origin_card, origin, target);
        }
        target->m_game->queue_request<request_indians>(origin_card, origin, target, flags);
    }
}