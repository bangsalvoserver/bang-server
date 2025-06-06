#include "poker.h"

#include "effects/base/pick.h"
#include "effects/base/escapable.h"
#include "effects/base/draw_check.h"

#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    struct request_poker : request_picking, escapable_request {
        request_poker(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags = {})
            : request_picking(origin_card, origin, target, flags, 100) {}

        void on_update() override {
            if (target->immune_to(origin_card, origin, flags)) {
                target->m_game->pop_request();
            } else {
                auto_pick();
            }
        }

        card_list get_highlights(player_ptr owner) const override {
            card_list result;
            target->m_game->call_event(event_type::get_selected_cards{ origin_card, owner, result });
            return result;
        }

        bool can_pick(const_card_ptr target_card) const override {
            return target_card->pocket == pocket_type::player_hand && target_card->owner == target;
        }

        void on_pick(card_ptr target_card) override {
            target->m_game->pop_request();

            target->m_game->add_listener<event_type::get_selected_cards>(origin_card,
                [origin_card=origin_card, target=target, target_card](card_ptr e_origin_card, player_ptr owner, card_list &result) {
                    if (origin_card == e_origin_card) {
                        if (owner == target) {
                            result.push_back(target_card);
                        } else {
                            for (card_ptr c : target->m_hand) {
                                result.push_back(c);
                            }
                        }
                    }
                });

            target->m_game->queue_action([=, origin_card=origin_card, target=target]{
                target->m_game->remove_listeners(origin_card);

                target->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, target, target_card);
                target_card->move_to(pocket_type::selection);
                target->m_game->call_event(event_type::on_discard_hand_card{ target, target_card, true });
            }, 95);
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
            : selection_picker(origin_card, nullptr, target, {}, 90) {}

        int num_cards = 2;

        void on_update() override {
            if (update_count != 0) return;
            
            if (auto aces = rv::filter(target->m_game->m_selection, [this](card_ptr c) {
                return get_modified_sign(c).rank == card_rank::rank_A;
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