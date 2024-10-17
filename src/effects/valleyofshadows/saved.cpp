#include "saved.h"

#include "effects/base/damage.h"
#include "effects/base/pick.h"

#include "game/game.h"
#include "game/bot_suggestion.h"

namespace banggame {
    
    struct request_saved : request_picking {
        request_saved(card_ptr origin_card, player_ptr target, player_ptr saved)
            : request_picking(origin_card, nullptr, target, {}, 0)
            , saved(saved) {}

        player_ptr saved = nullptr;

        void on_update() override {
            if (target->alive() && saved->alive()) {
                auto_pick();
            } else {
                target->m_game->pop_request();
            }
        }

        game_string pick_prompt(card_ptr target_card) const override {
            if (target->is_bot()
                && (bot_suggestion::is_target_enemy(target, saved)
                != (target_card->pocket == pocket_type::player_hand)))
            {
                return "BOT_BAD_PICK";
            } else {
                return {};
            }
        }

        bool can_pick(const_card_ptr target_card) const override {
            switch (target_card->pocket) {
            case pocket_type::player_hand:
                return target_card->owner == saved;
            case pocket_type::main_deck:
                return true;
            case pocket_type::discard_pile:
                return target->m_game->m_deck.empty();
            default:
                return false;
            }
        }

        void on_pick(card_ptr target_card) override {
            target->m_game->pop_request();
            if (target_card->pocket != pocket_type::player_hand) {
                target->draw_card(2, origin_card);
            } else {
                for (int i=0; i<2 && !saved->empty_hand(); ++i) {
                    card_ptr stolen_card = saved->random_hand_card();
                    if (stolen_card->visibility != card_visibility::shown) {
                        target->m_game->add_log(update_target::includes(target, saved), "LOG_STOLEN_CARD", target, saved, stolen_card);
                        target->m_game->add_log(update_target::excludes(target, saved), "LOG_STOLEN_CARD_FROM_HAND", target, saved);
                    } else {
                        target->m_game->add_log("LOG_STOLEN_CARD", target, saved, stolen_card);
                    }
                    target->steal_card(stolen_card);
                }
            }
        }

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_SAVED", origin_card, saved};
            } else {
                return {"STATUS_SAVED_OTHER", target, origin_card, saved};
            }
        }
    };

    bool effect_saved::can_play(card_ptr origin_card, player_ptr origin) {
        if (auto req = origin->m_game->top_request<request_damage>()) {
            return req->target != origin && (!req->savior || req->savior == origin);
        }
        return false;
    }

    void effect_saved::on_play(card_ptr origin_card, player_ptr origin) {
        auto req = origin->m_game->top_request<request_damage>();
        player_ptr saved = req->target;
        req->savior = origin;

        if (--req->damage == 0) {
            origin->m_game->pop_request();
        }

        origin->m_game->queue_request<request_saved>(origin_card, origin, saved);
    }
}