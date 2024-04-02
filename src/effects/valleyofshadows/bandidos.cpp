#include "bandidos.h"

#include "game/game.h"
#include "effects/base/requests.h"

namespace banggame {

    struct request_bandidos : request_resolvable, interface_picking {
        request_bandidos(card *origin_card, player *origin, player *target, effect_flags flags = {})
            : request_resolvable(origin_card, origin, target, flags) {}

        void on_update() override {
            if (target->immune_to(origin_card, origin, flags)) {
                target->m_game->pop_request();
            } else {
                if (!live) {
                    target->m_game->play_sound(target, "bandidos");
                }
                auto_resolve();
            }
        }

        void on_resolve() override {
            target->m_game->pop_request();
            target->damage(origin_card, origin, 1);
        }

        game_string resolve_prompt() const override {
            if (target->is_bot() && target->m_hp <= 1 && rn::any_of(target->m_hand, [&](card *target_card) { return can_pick(target_card); })) {
                return "BOT_BAD_PLAY";
            }
            return {};
        }
        
        bool can_pick(card *target_card) const override {
            return target_card->pocket == pocket_type::player_hand && target_card->owner == target;
        }

        void on_pick(card *target_card) override {
            target->m_game->pop_request();
            target->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, target, target_card);
            target->discard_used_card(target_card);
            if (!target->empty_hand()) {
                target->m_game->queue_request<request_discard>(origin_card, origin, target, effect_flags{}, 110);
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

    game_string effect_bandidos::on_prompt(card *origin_card, player *origin, const effect_context &ctx) {
        if (origin != ctx.skipped_player && origin->m_hp <= 1 && origin->m_hand.size() <= 1) {
            return {"PROMPT_BANDIDOS_SUICIDE", origin_card};
        }
        return {};
    }

    void effect_bandidos::on_play(card *origin_card, player *origin, player *target, effect_flags flags) {
        target->m_game->queue_request<request_bandidos>(origin_card, origin, target, flags);
    }
}