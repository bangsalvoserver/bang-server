#include "poker.h"

#include "game/game.h"

namespace banggame {

    struct request_poker : request_base {
        request_poker(card *origin_card, player *origin, player *target, effect_flags flags = {})
            : request_base(origin_card, origin, target, flags) {}

        void on_update() override {
            auto_pick();
        }

        bool can_pick(card *target_card) const override {
            return target_card->pocket == pocket_type::player_hand && target_card->owner == target;
        }

        void on_pick(card *target_card) override {
            target->m_game->invoke_action([&]{
                target->m_game->pop_request();
                target->m_game->add_log("LOG_DISCARDED_A_CARD_FOR", origin_card, target);
                target->m_game->move_card(target_card, pocket_type::selection, origin);
            });
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
            if (sent) return;
            
            for (card *target_card : target->m_game->m_selection) {
                target->m_game->add_log("LOG_POKER_REVEAL", origin_card, target_card);
                target->m_game->set_card_visibility(target_card);
            }
            
            if (auto it = std::ranges::find(target->m_game->m_selection, card_rank::rank_A, [this](card *c) {
                return target->get_card_sign(c).rank;
            }); it != target->m_game->m_selection.end()) {
                target->m_game->invoke_action([&]{
                    target->m_game->pop_request();
                    target->m_game->flash_card(*it);
                    target->m_game->add_short_pause();
                    target->m_game->add_log("LOG_POKER_ACE");
                    while (!target->m_game->m_selection.empty()) {
                        target->m_game->move_card(target->m_game->m_selection.front(), pocket_type::discard_pile);
                    }
                });
            } else if (target->m_game->m_selection.size() <= 2) {
                target->m_game->invoke_action([&]{
                    target->m_game->pop_request();
                    if (!target->m_game->m_selection.empty()) {
                        target->m_game->add_short_pause();
                    }
                    while (!target->m_game->m_selection.empty()) {
                        card *drawn_card = target->m_game->m_selection.front();
                        target->m_game->add_log("LOG_DRAWN_CARD", target, drawn_card);
                        target->add_to_hand(drawn_card);
                    }
                });
            }
        }

        void on_pick(card *target_card) override {
            target->m_game->invoke_action([&]{
                target->m_game->add_log("LOG_DRAWN_CARD", target, target_card);
                target->add_to_hand(target_card);
                if (--num_cards == 0) {
                    target->m_game->pop_request();
                    while (!target->m_game->m_selection.empty()) {
                        target->m_game->move_card(target->m_game->m_selection.front(), pocket_type::discard_pile);
                    }
                }
            });
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_POKER_DRAW", origin_card};
            } else {
                return {"STATUS_POKER_DRAW_OTHER", target, origin_card};
            }
        }
    };

    game_string effect_poker::on_prompt(card *origin_card, player *origin) {
        if (std::ranges::all_of(range_other_players(origin), &player::empty_hand)) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        }
        return {};
    }

    void effect_poker::on_play(card *origin_card, player *origin) {
        auto targets = to_vector(range_other_players(origin)
            | std::views::filter(std::not_fn(&player::empty_hand)));

        effect_flags flags = effect_flags::escapable;
        if (targets.size() == 1) flags |= effect_flags::single_target;
        
        for (player *p : targets) {
            origin->m_game->queue_request<request_poker>(origin_card, origin, p, flags);
        }
        origin->m_game->queue_request<request_poker_draw>(origin_card, origin);
    }
}