#include "poker.h"

#include "game/game.h"

namespace banggame {

    struct request_poker : request_base {
        request_poker(card *origin_card, player *origin, player *target, effect_flags flags = {})
            : request_base(origin_card, origin, target, flags | effect_flags::auto_pick) {}

        bool can_pick(card *target_card) const override {
            return target_card->pocket == pocket_type::player_hand && target_card->owner == target;
        }

        void on_pick(card *target_card) override {
            auto lock = target->m_game->lock_updates(true);
            target->m_game->add_log("LOG_DISCARDED_A_CARD_FOR", origin_card, target);
            target->m_game->move_card(target_card, pocket_type::selection, origin);
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

        void on_pick(card *target_card) override {
            auto lock = target->m_game->lock_updates();
            target->m_game->add_log("LOG_DRAWN_CARD", target, target_card);
            target->add_to_hand(target_card);
            if (--num_cards == 0 || target->m_game->m_selection.size() == 0) {
                target->m_game->pop_request();
                for (auto *c : target->m_game->m_selection) {
                    target->m_game->move_card(c, pocket_type::discard_pile);
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

    game_string effect_poker::on_prompt(card *origin_card, player *origin) {
        if (std::ranges::all_of(range_other_players(origin), [](const player &p) {
            return p.m_hand.empty();
        })) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        }
        return {};
    }

    void effect_poker::on_play(card *origin_card, player *origin) {
        std::vector<player *> targets;
        for (player &p : range_other_players(origin)) {
            if (!p.m_hand.empty()) {
                targets.push_back(&p);
            }
        }

        effect_flags flags = effect_flags::escapable;
        if (targets.size() == 1) flags |= effect_flags::single_target;
        
        for (player *p : targets) {
            origin->m_game->queue_request<request_poker>(origin_card, origin, p, flags);
        }
        origin->m_game->queue_action([=]{
            for (auto it = origin->m_game->m_selection.begin(); it != origin->m_game->m_selection.end(); ++it) {
                origin->m_game->add_log("LOG_POKER_REVEAL", origin_card, *it);
                auto flags = show_card_flags::shown;
                if (std::next(it) == origin->m_game->m_selection.end()) {
                    flags |= show_card_flags::short_pause;
                }
                origin->m_game->send_card_update(*it, nullptr, flags);
            }
            if (std::ranges::any_of(origin->m_game->m_selection, [origin](card *c) {
                return origin->get_card_sign(c).rank == card_rank::rank_A;
            })) {
                origin->m_game->add_log("LOG_POKER_ACE");
                while (!origin->m_game->m_selection.empty()) {
                    origin->m_game->move_card(origin->m_game->m_selection.front(), pocket_type::discard_pile);
                }
            } else if (origin->m_game->m_selection.size() <= 2) {
                while (!origin->m_game->m_selection.empty()) {
                    card *drawn_card = origin->m_game->m_selection.front();
                    origin->m_game->add_log("LOG_DRAWN_CARD", origin, drawn_card);
                    origin->add_to_hand(drawn_card);
                }
            } else {
                origin->m_game->queue_request<request_poker_draw>(origin_card, origin);
            }
        });
    }
}