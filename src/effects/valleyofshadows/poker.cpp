#include "poker.h"

#include "effects/base/pick.h"
#include "effects/base/resolve.h"

#include "game/game.h"

namespace banggame {

    struct request_poker : request_picking, escapable_request {
        request_poker(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags = {})
            : request_picking(origin_card, origin, target, flags) {}

        void on_update() override {
            auto_pick();
        }

        bool can_pick(const_card_ptr target_card) const override {
            return target_card->pocket == pocket_type::player_hand && target_card->owner == target;
        }

        void on_pick(card_ptr target_card) override {
            target->m_game->pop_request();
            if (!target->immune_to(origin_card, origin, flags)) {
                target->m_game->add_log(update_target::includes(origin, target), "LOG_DISCARDED_CARD_FOR", origin_card, target, target_card);
                target->m_game->add_log(update_target::excludes(origin, target), "LOG_DISCARDED_A_CARD_FOR", origin_card, target);
                target_card->move_to(pocket_type::selection, origin);
                target->m_game->call_event(event_type::on_discard_hand_card{ target, target_card, true });
            }
        }

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_POKER", origin_card};
            } else {
                return {"STATUS_POKER_OTHER", target, origin_card};
            }
        }
    };

    struct request_poker_draw : selection_picker {
        request_poker_draw(card_ptr origin_card, player_ptr target)
            : selection_picker(origin_card, nullptr, target) {}

        int num_cards = 2;

        void on_update() override {
            if (live) return;
            
            for (card_ptr target_card : target->m_game->m_selection) {
                target->m_game->add_log("LOG_POKER_REVEAL", origin_card, target_card);
                target_card->set_visibility(card_visibility::shown);
            }
            
            if (auto aces = rv::filter(target->m_game->m_selection, [this](card_ptr c) {
                return c->get_modified_sign().rank == card_rank::rank_A;
            })) {
                for (card_ptr c : aces) {
                    c->flash_card();
                }
                target->m_game->pop_request();
                target->m_game->add_short_pause();
                target->m_game->add_log("LOG_POKER_ACE");
                while (!target->m_game->m_selection.empty()) {
                    target->m_game->m_selection.front()->move_to(pocket_type::discard_pile);
                }
            } else if (target->m_game->m_selection.size() <= 2) {
                target->m_game->pop_request();
                if (!target->m_game->m_selection.empty()) {
                    target->m_game->add_short_pause();
                }
                while (!target->m_game->m_selection.empty()) {
                    card_ptr drawn_card = target->m_game->m_selection.front();
                    target->m_game->add_log("LOG_DRAWN_CARD", target, drawn_card);
                    target->add_to_hand(drawn_card);
                }
            }
        }

        void on_pick(card_ptr target_card) override {
            target->m_game->add_log("LOG_DRAWN_CARD", target, target_card);
            target->add_to_hand(target_card);
            if (--num_cards == 0) {
                target->m_game->pop_request();
                while (!target->m_game->m_selection.empty()) {
                    target->m_game->m_selection.front()->move_to(pocket_type::discard_pile);
                }
            }
        }

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_POKER_DRAW", origin_card};
            } else {
                return {"STATUS_POKER_DRAW_OTHER", target, origin_card};
            }
        }
    };

    void effect_poker::on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags) {
        origin->m_game->queue_request<request_poker>(origin_card, origin, target, flags);
    }

    void effect_poker::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->queue_request<request_poker_draw>(origin_card, origin);
    }
}