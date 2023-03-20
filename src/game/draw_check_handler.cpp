#include "draw_check_handler.h"

#include "game.h"

namespace banggame {

    void request_check::on_update() {
        if (state == request_state::pending) {
            start();
        }
    }

    bool request_check::can_pick(card *target_card) const {
        if (target_card->pocket == pocket_type::selection) {
            if (!target->is_bot()) return true;

            if (std::ranges::none_of(m_game->m_selection, [&](card *drawn_card) { return check(drawn_card); })) {
                return true;
            } else {
                return check(target_card);
            }
        }
        return false;
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
                card *drawn_card = m_game->top_of_deck();
                m_game->add_log("LOG_REVEALED_CARD", target, drawn_card);
                m_game->move_card(drawn_card, pocket_type::selection);
            }
        } else {
            card *drawn_card = m_game->top_of_deck();
            m_game->move_card(drawn_card, pocket_type::discard_pile);
            select(drawn_card);
        }
    }

    void request_check::select(card *drawn_card) {
        m_game->add_log("LOG_CHECK_DREW_CARD", origin_card, target, drawn_card);
        if (m_game->call_event<event_type::on_draw_check_select>(target, origin_card, drawn_card, true)) {
            resolve(drawn_card);
        }
    }

    void request_check::restart() {
        while (!m_game->m_selection.empty()) {
            m_game->move_card(m_game->m_selection.front(), pocket_type::discard_pile);
        }
        start();
    }

    bool request_check::check(card *drawn_card) const {
        return std::invoke(m_condition, m_game->get_card_sign(drawn_card));
    }

    void request_check::resolve(card *drawn_card) {
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
        std::invoke(m_function, check(drawn_card));
    }
}