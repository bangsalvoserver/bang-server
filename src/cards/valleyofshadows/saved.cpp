#include "saved.h"

#include "game/game.h"
#include "cards/base/damage.h"

namespace banggame {
    
    struct request_saved : request_base {
        request_saved(card *origin_card, player *target, player *saved)
            : request_base(origin_card, nullptr, target)
            , saved(saved) {}

        player *saved = nullptr;

        bool auto_resolve() override {
            return auto_pick();
        }

        bool can_pick(card *target_card) const override {
            return target_card->pocket == (target->m_game->m_deck.empty() ? pocket_type::discard_pile : pocket_type::main_deck)
                || target_card->pocket == pocket_type::player_hand && target_card->owner == saved;
        }

        void on_pick(card *target_card) override {
            auto lock = target->m_game->lock_updates(true);
            if (target_card->pocket != pocket_type::player_hand) {
                target->draw_card(2, origin_card);
            } else {
                for (int i=0; i<2 && !saved->m_hand.empty(); ++i) {
                    card *stolen_card = saved->random_hand_card();
                    target->m_game->add_log(update_target::includes(target, saved), "LOG_STOLEN_CARD", target, saved, stolen_card);
                    target->m_game->add_log(update_target::excludes(target, saved), "LOG_STOLEN_CARD_FROM_HAND", target, saved);
                    target->steal_card(stolen_card);
                }
            }
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_SAVED", origin_card, saved};
            } else {
                return {"STATUS_SAVED_OTHER", target, origin_card, saved};
            }
        }
    };

    bool effect_saved::can_respond(card *origin_card, player *origin) {
        if (auto *req = origin->m_game->top_request_if<request_damage>()) {
            return req->target != origin;
        }
        return false;
    }

    void effect_saved::on_play(card *origin_card, player *origin) {
        auto &req = origin->m_game->top_request().get<request_damage>();
        player *saved = req.target;

        auto lock = origin->m_game->lock_updates(--req.damage == 0);

        origin->m_game->queue_action([=]{
            if (saved->alive()) {
                origin->m_game->queue_request<request_saved>(origin_card, origin, saved);
            }
        }, 1);
    }
}