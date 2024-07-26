#include "draw_check.h"

#include "game/game.h"
#include "game/bot_suggestion.h"

namespace banggame {

    void request_check_base::on_update() {
        if (!live) {
            origin_card->flash_card();
            start();
        }
    }

    game_string request_check_base::pick_prompt(card_ptr target_card) const {
        if (target->is_bot() && !is_lucky(target_card)) {
            return "PROMPT_BAD_DRAW";
        } else {
            return {};
        }
    }

    void request_check_base::on_pick(card_ptr target_card) {
        target_card->flash_card();
        select(target_card);
    }

    game_string request_check_base::status_text(player_ptr owner) const {
        if (target == owner) {
            return {"STATUS_CHECK", origin_card};
        } else {
            return {"STATUS_CHECK_OTHER", target, origin_card};
        }
    }

    void request_check_base::start() {
        int num_checks = target->get_num_checks();
        if (num_checks > 1) {
            for (int i=0; i<num_checks; ++i) {
                card_ptr target_card = target->m_game->top_of_deck();
                target->m_game->add_log("LOG_REVEALED_CARD", target, target_card);
                target_card->move_to(pocket_type::selection);
            }
        } else {
            card_ptr target_card = target->m_game->top_of_deck();
            target_card->move_to(pocket_type::discard_pile);
            select(target_card);
        }
    }

    void request_check_base::select(card_ptr target_card) {
        drawn_card = target_card;

        target->m_game->add_log("LOG_CHECK_DREW_CARD", origin_card, target, target_card);
        bool handled = false;
        target->m_game->call_event(event_type::on_draw_check_select{ target, shared_from_this(), handled });
        if (!handled) {
            resolve();
        }
    }

    void request_check_base::restart() {
        while (!target->m_game->m_selection.empty()) {
            target->m_game->m_selection.front()->move_to(pocket_type::discard_pile);
        }
        start();
    }

    bool request_check_base::is_lucky(card_ptr target_card) const {
        return check_condition(target->m_game->get_card_sign(target_card));
    }

    bool request_check_base::bot_check_redraw(card_ptr target_card, player_ptr owner) const {
        return bot_suggestion::target_friend{}.on_check_target(target_card, owner, target) != is_lucky(drawn_card);
    }

    void request_check_base::resolve() {
        target->m_game->pop_request();
        if (!target->m_game->m_selection.empty()) {
            while (!target->m_game->m_selection.empty()) {
                card_ptr c = target->m_game->m_selection.front();
                target->m_game->call_event(event_type::on_draw_check_resolve{ target, c });
                if (c->pocket == pocket_type::selection) {
                    c->move_to(pocket_type::discard_pile);
                }
            }
        } else {
            target->m_game->call_event(event_type::on_draw_check_resolve{ target, drawn_card });
        }
        on_resolve(is_lucky(drawn_card));
    }
}