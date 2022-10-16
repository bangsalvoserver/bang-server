#include "bigfifty.h"

#include "../../game.h"
#include "../base/requests.h"

namespace banggame {

    void effect_bigfifty::on_play(card *origin_card, player *p) {
        p->m_game->add_listener<event_type::apply_bang_modifier>(origin_card, [=](player *origin, request_bang *req) {
            if (origin == p) {
                origin->m_game->add_disabler(origin_card, [target = req->target](card *c) {
                    return (c->pocket == pocket_type::player_table
                        || c->pocket == pocket_type::player_character)
                        && c->owner == target;
                });
                req->on_cleanup([=]{
                    p->m_game->remove_disablers(origin_card);
                });
                origin->m_game->remove_listeners(origin_card);
            }
        });
    }
}