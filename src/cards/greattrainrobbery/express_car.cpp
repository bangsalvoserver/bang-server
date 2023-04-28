#include "express_car.h"

#include "game/game.h"

#include "cards/base/requests.h"

#include "game/possible_to_play.h"

namespace banggame {

    game_string effect_express_car::on_prompt(card *origin_card, player *origin) {
        if (origin->is_bot()) {
            if (ranges::any_of(get_all_playable_cards(origin), [](card *c) { return c->pocket == pocket_type::player_hand; })) {
                return "BOT_BAD_PLAY";
            }
        } else if (auto ncards = origin->m_hand.size()) {
            if (ncards == 1) {
                return "PROMPT_PASS_DISCARD";
            } else {
                return {"PROMPT_PASS_DISCARD_PLURAL", int(ncards)};
            }
        }
        return {};
    }

    void effect_express_car::on_play(card *origin_card, player *origin) {
        ++origin->m_extra_turns;
        origin->m_game->queue_request<request_discard_hand>(origin_card, origin);
        origin->m_game->queue_action([=]{
            origin->pass_turn();
        });
    }
}