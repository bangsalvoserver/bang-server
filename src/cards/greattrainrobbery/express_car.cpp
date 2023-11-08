#include "express_car.h"

#include "game/game.h"

#include "cards/base/requests.h"

#include "game/possible_to_play.h"

namespace banggame {

    game_string effect_express_car::get_error(card *origin_card, player *origin) {
        if (origin->m_game->call_event<event_type::count_usages>(origin, origin_card, 0) >= 1) {
            return {"ERROR_MAX_USAGES", origin_card, 1};
        }
        return {};
    }

    game_string effect_express_car::on_prompt(card *origin_card, player *origin) {
        if (origin->is_bot()) {
            if (ranges::any_of(get_all_playable_cards(origin), [](card *c) { return c->pocket == pocket_type::player_hand; })) {
                return "BOT_BAD_PLAY";
            }
        } else if (int ncards = int(origin->m_hand.size())) {
            return {"PROMPT_PASS_DISCARD", ncards};
        }
        return {};
    }

    void effect_express_car::on_play(card *origin_card, player *origin) {
        event_card_key key{origin_card, 5};
        origin->m_game->add_listener<event_type::count_usages>(key, [=](player *e_origin, card *e_card, int &usages) {
            if (origin_card == e_card && origin == e_origin) {
                ++usages;
            }
        });
        origin->m_game->add_listener<event_type::pre_turn_start>(key, [=](player *p) {
            if (p != origin) {
                origin->m_game->remove_listeners(key);
            }
        });

        ++origin->m_extra_turns;
        origin->m_game->queue_request<request_discard_hand>(origin_card, origin);
        origin->m_game->queue_action([=]{
            origin->pass_turn();
        });
    }
}