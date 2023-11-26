#include "poker.h"

#include "game/game.h"

namespace banggame {

    struct request_poker : request_picking {
        request_poker(card *origin_card, player *origin, player *target, effect_flags flags = {})
            : request_picking(origin_card, origin, target, flags) {}

        void on_update() override {
            auto_pick();
        }

        bool can_pick(card *target_card) const override {
            return target_card->pocket == pocket_type::player_hand && target_card->owner == target;
        }

        void on_pick(card *target_card) override {
            target->m_game->pop_request();
            target->m_game->add_log(update_target::includes(origin, target), "LOG_DISCARDED_CARD_FOR", origin_card, target, target_card);
            target->m_game->add_log(update_target::excludes(origin, target), "LOG_DISCARDED_A_CARD_FOR", origin_card, target);
            target->m_game->move_card(target_card, pocket_type::selection, origin);
            target->m_game->call_event<event_type::on_discard_hand_card>(target, target_card, true);
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_POKER", origin_card};
            } else {
                return {"STATUS_POKER_OTHER", target, origin_card};
            }
        }
    };

    struct request_poker_draw : selection_picker {
        request_poker_draw(card *origin_card, player *target)
            : selection_picker(origin_card, nullptr, target) {}

        int num_cards = 2;

        void on_update() override {
            if (live) return;
            
            for (card *target_card : target->m_game->m_selection) {
                target->m_game->add_log("LOG_POKER_REVEAL", origin_card, target_card);
                target->m_game->set_card_visibility(target_card);
            }
            
            if (auto aces = std::views::filter(target->m_game->m_selection, [this](card *c) {
                return target->m_game->get_card_sign(c).rank == card_rank::rank_A;
            })) {
                for (card *c : aces) {
                    target->m_game->flash_card(c);
                }
                target->m_game->pop_request();
                target->m_game->add_short_pause();
                target->m_game->add_log("LOG_POKER_ACE");
                while (!target->m_game->m_selection.empty()) {
                    target->m_game->move_card(target->m_game->m_selection.front(), pocket_type::discard_pile);
                }
            } else if (target->m_game->m_selection.size() <= 2) {
                target->m_game->pop_request();
                if (!target->m_game->m_selection.empty()) {
                    target->m_game->add_short_pause();
                }
                while (!target->m_game->m_selection.empty()) {
                    card *drawn_card = target->m_game->m_selection.front();
                    target->m_game->add_log("LOG_DRAWN_CARD", target, drawn_card);
                    target->add_to_hand(drawn_card);
                }
            }
        }

        void on_pick(card *target_card) override {
            target->m_game->add_log("LOG_DRAWN_CARD", target, target_card);
            target->add_to_hand(target_card);
            if (--num_cards == 0) {
                target->m_game->pop_request();
                while (!target->m_game->m_selection.empty()) {
                    target->m_game->move_card(target->m_game->m_selection.front(), pocket_type::discard_pile);
                }
            }
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_POKER_DRAW", origin_card};
            } else {
                return {"STATUS_POKER_DRAW_OTHER", target, origin_card};
            }
        }
    };

    void effect_poker::on_play(card *origin_card, player *origin, player *target, effect_flags flags) {
        origin->m_game->queue_request<request_poker>(origin_card, origin, target, flags);
    }

    void effect_poker::on_play(card *origin_card, player *origin) {
        origin->m_game->queue_request<request_poker_draw>(origin_card, origin);
    }
}