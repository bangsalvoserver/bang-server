#include "draw_check_handler.h"

#include "game.h"

#include "game/bot_suggestion.h"

namespace banggame {

    void request_check::on_update() {
        if (!live) {
            m_game->flash_card(origin_card);
            start();
        }
    }

    game_string request_check::pick_prompt(card *target_card) const {
        if (target->is_bot() && !is_lucky(target_card)) {
            return "PROMPT_BAD_DRAW";
        } else {
            return {};
        }
    }

    void request_check::on_pick(card *target_card) {
        m_game->flash_card(target_card);
        select(target_card);
    }

    game_string request_check::status_text(player *owner) const {
        if (target == owner) {
            return {"STATUS_CHECK", origin_card};
        } else {
            return {"STATUS_CHECK_OTHER", target, origin_card};
        }
    }

    void request_check::start() {
        int num_checks = target ? target->get_num_checks() : 1;
        if (num_checks > 1) {
            for (int i=0; i<num_checks; ++i) {
                card *target_card = m_game->top_of_deck();
                m_game->add_log("LOG_REVEALED_CARD", target, target_card);
                m_game->move_card(target_card, pocket_type::selection);
            }
        } else {
            card *target_card = m_game->top_of_deck();
            m_game->move_card(target_card, pocket_type::discard_pile);
            select(target_card);
        }
    }

    void request_check::select(card *target_card) {
        drawn_card = target_card;

        m_game->add_log("LOG_CHECK_DREW_CARD", origin_card, target, target_card);
        if (!m_game->call_event<event_type::on_draw_check_select>(target, shared_from_this(), false)) {
            resolve();
        }
    }

    void request_check::restart() {
        while (!m_game->m_selection.empty()) {
            m_game->move_card(m_game->m_selection.front(), pocket_type::discard_pile);
        }
        start();
    }

    bool request_check::is_lucky(card *target_card) const {
        return check_condition(m_game->get_card_sign(target_card));
    }

    bool request_check::bot_check_redraw(card *target_card, player *owner) const {
        return (target && bot_suggestion::target_friend{}.on_check_target(target_card, owner, target)) != is_lucky(drawn_card);
    }

    void request_check::resolve() {
        m_game->pop_request();
        if (!m_game->m_selection.empty()) {
            while (!m_game->m_selection.empty()) {
                card *c = m_game->m_selection.front();
                m_game->call_event<event_type::on_draw_check_resolve>(target, c);
                if (c->pocket == pocket_type::selection) {
                    m_game->move_card(c, pocket_type::discard_pile);
                }
            }
        } else {
            m_game->call_event<event_type::on_draw_check_resolve>(target, drawn_card);
        }
        on_resolve(is_lucky(drawn_card));
    }
}