#include "draw.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"

#include "game/game.h"
#include "game/possible_to_play.h"

namespace banggame {

    void effect_draw::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        target->draw_card(ncards, origin_card);
    }

    void effect_queue_draw::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        origin->m_game->queue_action([=, ncards=ncards]{
            target->draw_card(ncards, origin_card);
        });
    }

    game_string effect_draw_discard::get_error(card_ptr origin_card, player_ptr origin, player_ptr target) {
        if (target->m_game->m_discards.empty()) {
            return "ERROR_DISCARD_PILE_EMPTY";
        }
        if (target->m_game->check_flags(game_flag::phase_one_draw_discard)) {
            return "ERROR_INVALID_ACTION";
        }
        return {};
    }
    
    void effect_draw_discard::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        card_ptr drawn_card = target->m_game->m_discards.back();
        target->m_game->add_log("LOG_DRAWN_FROM_DISCARD", target, drawn_card);
        target->add_to_hand(drawn_card);
    }

    void effect_draw_to_discard::on_play(card_ptr origin_card, player_ptr origin) {
        for (int i=0; i<ncards; ++i) {
            origin->m_game->top_of_deck()->move_to(pocket_type::discard_pile);
        }
    }

    request_draw::request_draw(player_ptr target)
        : request_picking(nullptr, nullptr, target, {}, -7)
    {
        target->m_game->call_event(event_type::count_cards_to_draw{ target, num_cards_to_draw });
    }
    
    void request_draw::on_update() {
        cards_from_selection = target->m_game->m_selection;
        if (!target->m_game->check_flags(game_flag::phase_one_override)
            && target->alive() && target->m_game->m_playing == target
            && num_drawn_cards < num_cards_to_draw)
        {
            if (!live) {
                target->play_sound("draw");
            }
            card_ptr only_card = get_single_element(get_all_playable_cards(target, true));
            if (only_card && only_card->has_tag(tag_type::pick)) {
                on_pick(nullptr);
            } else {
                cleanup_selection();
            }
        } else {
            target->m_game->pop_request();
            cleanup_selection();
        }
    }

    void request_draw::cleanup_selection() {
        for (card_ptr target_card : cards_from_selection) {
            target_card->move_to(pocket_type::main_deck, nullptr, card_visibility::hidden);
        }
    }

    bool request_draw::can_pick(const_card_ptr target_card) const {
        if (target->m_game->m_deck.empty() || target->m_game->check_flags(game_flag::phase_one_draw_discard) && !target->m_game->m_discards.empty()) {
            return target_card->pocket == pocket_type::discard_pile;
        } else {
            return target_card->pocket == pocket_type::main_deck;
        }
    }

    card_ptr request_draw::phase_one_drawn_card() {
        if (!target->m_game->check_flags(game_flag::phase_one_draw_discard) || target->m_game->m_discards.empty()) {
            if (!cards_from_selection.empty()) {
                card_ptr target_card = cards_from_selection.front();
                cards_from_selection.erase(cards_from_selection.begin());
                return target_card;
            }
            return target->m_game->top_of_deck();
        } else {
            return target->m_game->m_discards.back();
        }
    }

    void request_draw::add_to_hand_phase_one(card_ptr drawn_card) {
        ++num_drawn_cards;
        
        bool reveal = false;
        target->m_game->call_event(event_type::on_card_drawn{ target, drawn_card, shared_from_this(), reveal });
        if (drawn_card->pocket == pocket_type::discard_pile) {
            target->m_game->add_log("LOG_DRAWN_FROM_DISCARD", target, drawn_card);
        } else if (target->m_game->check_flags(game_flag::hands_shown)) {
            target->m_game->add_log("LOG_DRAWN_CARD", target, drawn_card);
        } else if (reveal) {
            target->m_game->add_log("LOG_DRAWN_CARD", target, drawn_card);
            drawn_card->set_visibility(card_visibility::shown);
            drawn_card->add_short_pause();
        } else {
            target->m_game->add_log(update_target::excludes(target), "LOG_DRAWN_CARDS", target, 1);
            target->m_game->add_log(update_target::includes(target), "LOG_DRAWN_CARD", target, drawn_card);
        }
        target->add_to_hand(drawn_card);
    }

    void request_draw::on_pick(card_ptr target_card) {
        target->m_game->pop_request();
        bool handled = false;
        target->m_game->call_event(event_type::on_draw_from_deck{ target, shared_from_this(), handled });
        if (!handled) {
            while (num_drawn_cards < num_cards_to_draw) {
                add_to_hand_phase_one(phase_one_drawn_card());
            }
            cleanup_selection();
        }
    }

    game_string request_draw::status_text(player_ptr owner) const {
        if (owner == target) {
            return "STATUS_YOUR_TURN";
        } else {
            return {"STATUS_YOUR_TURN_OTHER", target};
        }
    }
    
    bool effect_while_drawing::can_play(card_ptr origin_card, player_ptr origin) {
        return origin->m_game->top_request<request_draw>(origin) != nullptr;
    }
    
    bool effect_no_cards_drawn::can_play(card_ptr origin_card, player_ptr origin) {
        auto req = origin->m_game->top_request<request_draw>(origin);
        return req && req->num_drawn_cards == 0;
    }

    void effect_add_draw_card::on_play(card_ptr origin_card, player_ptr origin) {
        ++origin->m_game->top_request<request_draw>()->num_drawn_cards;
    }

    void effect_skip_drawing::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->top_request<request_draw>()->cleanup_selection();
        origin->m_game->pop_request();
    }
}