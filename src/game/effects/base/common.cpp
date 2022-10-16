#include "common.h"

#include "../../game.h"

namespace banggame {
    
    void effect_play_card_action::on_play(card *origin_card, player *origin) {
        origin->play_card_action(origin_card);
    }

    game_string effect_max_usages::verify(card *origin_card, player *origin) {
        if (origin->m_game->call_event<event_type::count_usages>(origin, origin_card, 0) >= max_usages) {
            return {"ERROR_MAX_USAGES", origin_card, max_usages};
        }
        return {};
    }
    
    void effect_max_usages::on_play(card *origin_card, player *origin) {
        event_card_key key{origin_card, 5};
        origin->m_game->add_listener<event_type::count_usages>(key, [=](player *e_origin, card *e_card, int &usages) {
            if (origin_card == e_card && origin == e_origin) {
                ++usages;
            }
        });
        origin->m_game->add_listener<event_type::on_turn_end>(key, [=](player *e_origin, bool skipped) {
            if (e_origin == origin) {
                origin->m_game->remove_listeners(key);
            }
        });
    }

    game_string effect_pass_turn::on_prompt(card *origin_card, player *origin) {
        int diff = static_cast<int>(origin->m_hand.size()) - origin->max_cards_end_of_turn();
        if (diff == 1) {
            return "PROMPT_PASS_DISCARD";
        } else if (diff > 1) {
            return {"PROMPT_PASS_DISCARD_PLURAL", diff};
        }
        return {};
    }

    game_string effect_pass_turn::verify(card *origin_card, player *origin) {
        return origin->m_game->call_event<event_type::verify_pass_turn>(origin, game_string{});
    }

    void effect_pass_turn::on_play(card *origin_card, player *origin) {
        origin->pass_turn();
    }

    bool effect_resolve::can_respond(card *origin_card, player *origin) {
        return origin->m_game->top_request_is<resolvable_request>(origin);
    }
    
    void effect_resolve::on_play(card *origin_card, player *origin) {
        auto copy = origin->m_game->top_request();
        copy.get<resolvable_request>().on_resolve();
    }
    
    void event_based_effect::on_disable(card *target_card, player *target) {
        target->m_game->remove_listeners(target_card);
    }
}