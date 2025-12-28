#include "draw_check.h"

#include "game/game_table.h"
#include "game/bot_suggestion.h"

namespace banggame {
    
    card_sign get_modified_sign(const_card_ptr target_card) {
        auto value = target_card->sign;
        target_card->m_game->call_event(event_type::apply_sign_modifier{ value });
        return value;
    }

    void request_check_base::on_update() {
        if (update_count == 0) {
            origin_card->flash_card();
            start();
        }
    }

    prompt_string request_check_base::pick_prompt(card_ptr target_card) const {
        if (target->is_bot()) {
            auto result = get_result(target_card);
            if (!result.lucky && !result.indifferent) {
                return "BOT_BAD_DRAW";
            }
        }
        return {};
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
        bool handled = false;
        target->m_game->call_event(event_type::on_draw_check_start{ target, shared_from_this(), handled });
        if (!handled) {
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

    prompt_string request_check_base::redraw_prompt(card_ptr target_card, player_ptr owner) const {
        if (owner->is_bot()) {
            draw_check_result result = get_result(drawn_card);
            if (result.indifferent) {
                return "BOT_DONT_REDRAW_INDIFFERENT";
            } else if (result.defensive_redraw) {
                if (result.lucky || !bot_suggestion::is_target_friend(owner, target)) {
                    return "BOT_DONT_REDRAW_DEFENSIVE";
                }
            } else {
                if (result.lucky == bot_suggestion::is_target_friend(owner, target)) {
                    return "BOT_DONT_REDRAW_AGGRESSIVE";
                }
            }
        }
        return {};
    }

    void request_check_base::resolve() {
        target->m_game->pop_request();
        if (!target->m_game->m_selection.empty()) {
            while (!target->m_game->m_selection.empty()) {
                card_ptr c = target->m_game->m_selection.front();
                target->m_game->call_event(event_type::on_draw_check_resolve{ origin_card, target, c, drawn_card });
                if (c->pocket == pocket_type::selection) {
                    c->move_to(pocket_type::discard_pile);
                }
            }
        } else {
            target->m_game->call_event(event_type::on_draw_check_resolve{origin_card, target, drawn_card, drawn_card });
        }
        on_resolve(get_result(drawn_card).lucky);
    }
}