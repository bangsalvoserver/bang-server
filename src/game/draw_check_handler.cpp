#include "draw_check_handler.h"

#include "game.h"

namespace banggame {

    struct request_check : request_base {
        request_check(card *origin_card, player *target)
            : request_base(origin_card, nullptr, target) {}

        bool can_pick(card *target_card) const override {
            if (target_card->pocket == pocket_type::selection) {
                if (!target->is_bot()) return true;

                auto is_lucky = [&m_current_check = target->m_game->m_current_check](card *drawn_card) {
                    return m_current_check.check(drawn_card);
                };

                if (std::ranges::none_of(target->m_game->m_selection, is_lucky)) {
                    return true;
                } else {
                    return is_lucky(target_card);
                }
            }
            return false;
        }

        void on_pick(card *target_card) override {
            target->m_game->invoke_action([&]{
                target->m_game->pop_request();
                target->m_game->flash_card(target_card);
                target->m_game->m_current_check.select(target_card);
            });
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_CHECK", origin_card};
            } else {
                return {"STATUS_CHECK_OTHER", target, origin_card};
            }
        }
    };

    void draw_check_handler::set(player *origin, card *origin_card, draw_check_condition &&condition, draw_check_function &&function) {
        m_origin = origin;
        m_origin_card = origin_card;
        m_condition = std::move(condition);
        m_function = std::move(function);
    }

    void draw_check_handler::start() {
        int num_checks = m_origin->get_num_checks();
        if (num_checks > 1) {
            for (int i=0; i<num_checks; ++i) {
                card *drawn_card = m_origin->m_game->top_of_deck();
                m_origin->m_game->add_log("LOG_REVEALED_CARD", m_origin, drawn_card);
                m_origin->m_game->move_card(drawn_card, pocket_type::selection);
            }
            m_origin->m_game->queue_request_front<request_check>(m_origin_card, m_origin);
        } else {
            card *drawn_card = m_origin->m_game->top_of_deck();
            m_origin->m_game->move_card(drawn_card, pocket_type::discard_pile);
            select(drawn_card);
        }
    }

    void draw_check_handler::select(card *drawn_card) {
        m_origin->m_game->add_log("LOG_CHECK_DREW_CARD", m_origin_card, m_origin, drawn_card);
        if (m_origin->m_game->call_event<event_type::on_draw_check_select>(m_origin, m_origin_card, drawn_card, true)) {
            resolve(drawn_card);
        }
    }

    void draw_check_handler::restart() {
        while (!m_origin->m_game->m_selection.empty()) {
            m_origin->m_game->move_card(m_origin->m_game->m_selection.front(), pocket_type::discard_pile);
        }
        start();
    }

    bool draw_check_handler::check(card *drawn_card) {
        return std::invoke(m_condition, m_origin->get_card_sign(drawn_card));
    }

    void draw_check_handler::resolve(card *drawn_card) {
        if (m_origin->get_num_checks() > 1) {
            while (!m_origin->m_game->m_selection.empty()) {
                card *c = m_origin->m_game->m_selection.front();
                m_origin->m_game->call_event<event_type::on_draw_check_resolve>(m_origin, c);
                if (c->pocket == pocket_type::selection) {
                    m_origin->m_game->move_card(c, pocket_type::discard_pile);
                }
            }
        } else {
            m_origin->m_game->call_event<event_type::on_draw_check_resolve>(m_origin, drawn_card);
        }
        bool result = check(drawn_card);
        m_origin = nullptr;
        m_origin_card = nullptr;
        m_condition = nullptr;
        std::invoke(std::exchange(m_function, nullptr), result);
    }
}