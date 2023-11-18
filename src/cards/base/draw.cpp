#include "draw.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    void effect_draw::on_play(card *origin_card, player *origin, player *target) {
        origin->m_game->queue_action([=, ncards=ncards]{
            target->draw_card(ncards, origin_card);
        }, 200);
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
        if (target->alive() && target->m_game->m_playing == target && target->m_num_drawn_cards < target->get_cards_to_draw()) {
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

    void request_draw::on_pick(card *target_card) {
        bool handled = target->m_game->call_event<event_type::on_draw_from_deck>(target, false);
        if (!handled) {
            target->m_game->pop_request();
            int ncards = target->get_cards_to_draw();
            while (target->m_num_drawn_cards < ncards) {
                target->add_to_hand_phase_one(target->m_game->phase_one_drawn_card());
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
        if (origin->m_num_drawn_cards != 0) {
            return "ERROR_NOT_START_OF_TURN";
        }
        return {};
    }
    
    bool effect_while_drawing::can_play(card *origin_card, player *origin) {
        return origin->m_game->top_request<request_draw>(origin) != nullptr;
    }

    void effect_while_drawing::on_play(card *origin_card, player *origin) {
        if (cards_to_add != 0) {
            if (cards_to_add > 0) {
                origin->m_num_drawn_cards += cards_to_add;
            } else {
                origin->m_game->pop_request();
            }
        }
    }
}