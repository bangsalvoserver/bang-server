#include "escape.h"

#include "effects/base/escapable.h"

#include "cards/game_enums.h"

#include "game/game_table.h"

namespace banggame {

    prompt_string effect_escape_base::on_prompt(card_ptr origin_card, player_ptr origin) {
        auto req = origin->m_game->top_request<escapable_request>();
        return req->escape_prompt(origin);
    }

    void effect_escape_base::on_play(card_ptr origin_card, player_ptr origin) {
        auto req = origin->m_game->top_request<escapable_request>();
        req->add_card(origin_card);
        origin->m_game->pop_request();
    }

    bool effect_escape::can_escape(player_ptr origin, card_ptr origin_card, effect_flags flags) {
        return origin && origin_card
            && origin_card->is_brown()
            && !flags.check(effect_flag::is_bang);
    }
    
    bool effect_escape::can_play(card_ptr origin_card, player_ptr origin) {
        if (auto req = origin->m_game->top_request<escapable_request>([&](const request_base &base) {
            return base.target == origin && effect_escape::can_escape(base.origin, base.origin_card, base.flags);
        })) {
            return req->can_escape(origin_card);
        }
        return false;
    }

    bool effect_escape2::can_escape(player_ptr origin, card_ptr origin_card, effect_flags flags) {
        return origin && origin_card
            && (origin_card->deck == card_deck_type::main_deck
                || origin_card->deck == card_deck_type::goldrush
                || origin_card->deck == card_deck_type::train)
            && !flags.check(effect_flag::is_bang)
            && flags.check(effect_flag::single_target)
            && !flags.check(effect_flag::multi_target);
    }
    
    bool effect_escape2::can_play(card_ptr origin_card, player_ptr origin) {
        if (auto req = origin->m_game->top_request<escapable_request>([&](const request_base &base) {
            return base.target == origin && effect_escape2::can_escape(base.origin, base.origin_card, base.flags);
        })) {
            return req->can_escape(origin_card);
        }
        return false;
    }
}