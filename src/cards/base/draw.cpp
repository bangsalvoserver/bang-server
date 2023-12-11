#include "draw.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    void effect_draw::on_play(card *origin_card, player *origin, player *target) {
        target->draw_card(ncards, origin_card);
    }

    void effect_queue_draw::on_play(card *origin_card, player *origin, player *target) {
        origin->m_game->queue_action([=, ncards=ncards]{
            target->draw_card(ncards, origin_card);
        });
    }

    void handler_draw_multi::on_play(card *origin_card, player *origin, int amount) {
        if (amount > 0) {
            effect_draw{amount}.on_play(origin_card, origin);
        }
    }

    game_string effect_draw_discard::get_error(card *origin_card, player *origin, player *target) {
        if (target->m_game->m_discards.empty()) {
            return "ERROR_DISCARD_PILE_EMPTY";
        }
        if (target->m_game->check_flags(game_flags::phase_one_draw_discard)) {
            return "ERROR_INVALID_ACTION";
        }
        return {};
    }
    
    void effect_draw_discard::on_play(card *origin_card, player *origin, player *target) {
        card *drawn_card = target->m_game->m_discards.back();
        target->m_game->add_log("LOG_DRAWN_FROM_DISCARD", target, drawn_card);
        target->add_to_hand(drawn_card);
    }

    void effect_draw_to_discard::on_play(card *origin_card, player *origin) {
        for (int i=0; i<ncards; ++i) {
            origin->m_game->move_card(origin->m_game->top_of_deck(), pocket_type::discard_pile);
        }
    }
    
    void request_draw::on_update() {
        if (!target->m_game->check_flags(game_flags::phase_one_override)
            && target->alive() && target->m_game->m_playing == target
            && num_drawn_cards < target->get_cards_to_draw())
        {
            if (!live) {
                target->m_game->play_sound(target, "draw");
            }
            auto_pick();
        } else {
            target->m_game->pop_request();
        }
    }

    bool request_draw::can_pick(card *target_card) const {
        if (target->m_game->m_deck.empty() || target->m_game->check_flags(game_flags::phase_one_draw_discard) && !target->m_game->m_discards.empty()) {
            return target_card->pocket == pocket_type::discard_pile;
        } else {
            return target_card->pocket == pocket_type::main_deck;
        }
    }

    card *request_draw::phase_one_drawn_card() {
        if (!target->m_game->check_flags(game_flags::phase_one_draw_discard) || target->m_game->m_discards.empty()) {
            return target->m_game->top_of_deck();
        } else {
            return target->m_game->m_discards.back();
        }
    }

    void request_draw::add_to_hand_phase_one(card *drawn_card) {
        ++num_drawn_cards;
        
        bool reveal = false;
        target->m_game->call_event(event_type::on_card_drawn{ target, drawn_card, shared_from_this(), reveal });
        if (drawn_card->pocket == pocket_type::discard_pile) {
            target->m_game->add_log("LOG_DRAWN_FROM_DISCARD", target, drawn_card);
        } else if (target->m_game->check_flags(game_flags::hands_shown)) {
            target->m_game->add_log("LOG_DRAWN_CARD", target, drawn_card);
        } else if (reveal) {
            target->m_game->add_log("LOG_DRAWN_CARD", target, drawn_card);
            target->m_game->set_card_visibility(drawn_card);
            target->m_game->add_short_pause(drawn_card);
        } else {
            target->m_game->add_log(update_target::excludes(target), "LOG_DRAWN_CARDS", target, 1);
            target->m_game->add_log(update_target::includes(target), "LOG_DRAWN_CARD", target, drawn_card);
        }
        target->add_to_hand(drawn_card);
    }

    void request_draw::on_pick(card *target_card) {
        target->m_game->pop_request();
        bool handled = false;
        target->m_game->call_event(event_type::on_draw_from_deck{ target, shared_from_this(), handled });
        if (!handled) {
            int ncards = target->get_cards_to_draw();
            while (num_drawn_cards < ncards) {
                add_to_hand_phase_one(phase_one_drawn_card());
            }
        }
    }

    game_string request_draw::status_text(player *owner) const {
        if (owner == target) {
            return "STATUS_YOUR_TURN";
        } else {
            return {"STATUS_YOUR_TURN_OTHER", target};
        }
    }
    
    game_string effect_startofturn::get_error(card *origin_card, player *origin) const {
        if (auto req = origin->m_game->top_request<request_draw>(origin)) {
            if (req->num_drawn_cards == 0) {
                return {};
            }
        }
        return "ERROR_NOT_START_OF_TURN";
    }
    
    bool effect_while_drawing::can_play(card *origin_card, player *origin) {
        return origin->m_game->top_request<request_draw>(origin) != nullptr;
    }

    void effect_while_drawing::on_play(card *origin_card, player *origin) {
        if (cards_to_add != 0) {
            if (cards_to_add > 0) {
                origin->m_game->top_request<request_draw>(origin)->num_drawn_cards += cards_to_add;
            } else {
                origin->m_game->pop_request();
            }
        }
    }
}