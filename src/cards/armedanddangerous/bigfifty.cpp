#include "bigfifty.h"

#include "game/game.h"
#include "cards/effect_context.h"

namespace banggame {

    void effect_bigfifty::on_play(card *origin_card, player *p) {
        event_card_key key{origin_card, 1};
        p->m_game->add_listener<event_type::apply_bang_modifier>(key, [=](player *origin, request_bang *req) {
            if (origin == p) {
                origin->m_game->add_disabler(key, [target = req->target](card *c) {
                    return (c->pocket == pocket_type::player_table
                        || c->pocket == pocket_type::player_character)
                        && c->owner == target;
                });
                req->on_cleanup([=]{
                    p->m_game->remove_disablers(key);
                });
                origin->m_game->remove_listeners(key);
            }
        });
    }

    void modifier_bigfifty::add_context(card *origin_card, player *origin, effect_context &ctx) {
        ctx.disable_bang_checks = true;
    }
}