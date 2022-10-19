#include "draw_check_handler.h"

#include "game.h"

namespace banggame {

    struct request_check : selection_picker {
        request_check(card *origin_card, player *target)
            : selection_picker(origin_card, nullptr, target) {}

        void on_pick(pocket_type pocket, player *target_player, card *target_card) override {
            target->m_game->flash_card(target_card);
            target->m_game->pop_request();
            target->m_game->m_current_check.select(target_card);
            target->m_game->update_request();
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_CHECK", origin_card};
            } else {
                return {"STATUS_CHECK_OTHER", target, origin_card};
            }
        }
    };

    void draw_check_handler::set(player *origin, card *origin_card, draw_check_function &&function) {
        m_origin = origin;
        m_origin_card = origin_card;
        m_function = std::move(function);
    }

    void draw_check_handler::start() {
        int num_checks = m_origin->get_num_checks();
        if (num_checks > 1) {
            for (int i=0; i<num_checks; ++i) {
                m_origin->m_game->add_log("LOG_REVEALED_CARD", m_origin, m_origin->m_game->m_deck.back());
                m_origin->m_game->draw_card_to(pocket_type::selection, nullptr);
            }
            m_origin->m_game->queue_request_front<request_check>(m_origin_card, m_origin);
        } else {
            select(m_origin->m_game->draw_card_to(pocket_type::discard_pile));
        }
    }

    void draw_check_handler::select(card *drawn_card) {
        m_origin->m_game->add_log("LOG_CHECK_DREW_CARD", m_origin_card, m_origin, drawn_card);
        if (!m_origin->m_game->num_queued_requests([&]{
            m_origin->m_game->call_event<event_type::on_draw_check_select>(m_origin, m_origin_card, drawn_card);
        })) {
            resolve(drawn_card);
        }
    }

    void draw_check_handler::restart() {
        while (!m_origin->m_game->m_selection.empty()) {
            m_origin->m_game->move_card(m_origin->m_game->m_selection.front(), pocket_type::discard_pile);
        }
        start();
    }

    void draw_check_handler::resolve(card *drawn_card) {
        if (m_origin->get_num_checks() > 1) {
            while (!m_origin->m_game->m_selection.empty()) {
                card *c = m_origin->m_game->m_selection.front();
                m_origin->m_game->call_event<event_type::on_draw_check>(m_origin, c);
                if (c->pocket == pocket_type::selection) {
                    m_origin->m_game->move_card(c, pocket_type::discard_pile, nullptr);
                }
            }
        } else {
            m_origin->m_game->call_event<event_type::on_draw_check>(m_origin, drawn_card);
        }
        m_origin = nullptr;
        m_origin_card = nullptr;
        std::invoke(std::exchange(m_function, nullptr), m_origin->get_card_sign(drawn_card));
    }
}