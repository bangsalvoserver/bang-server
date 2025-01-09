#include "doublebarrel.h"

#include "game/game_table.h"
#include "effects/base/bang.h"

namespace banggame {

    void effect_doublebarrel::on_play(card_ptr origin_card, player_ptr origin) {
        event_card_key key{origin_card, 1};
        origin->m_game->add_listener<event_type::apply_bang_modifier>(key, [=](player_ptr p, shared_request_bang req) {
            if (p == origin) {
                if (req->origin_card->get_modified_sign().is_diamonds()) {
                    req->unavoidable = true;
                }
                origin->m_game->remove_listeners(key);
            }
        });
    }

    game_string effect_doublebarrel::on_prompt(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        card_sign sign = ctx.playing_card->get_modified_sign();
        if (sign && !sign.is_diamonds()) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        } else {
            return {};
        }
    }
}