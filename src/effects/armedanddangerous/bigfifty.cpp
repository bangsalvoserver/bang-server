#include "bigfifty.h"

#include "game/game_table.h"

namespace banggame {

    void effect_bigfifty::on_play(card_ptr origin_card, player_ptr p) {
        event_card_key key{origin_card, 1};
        p->m_game->add_listener<event_type::apply_bang_modifier>(key, [=](player_ptr origin, shared_request_bang req) {
            if (origin == p) {
                origin->m_game->add_disabler(key, [target = req->target](const_card_ptr c) {
                    return (c->pocket == pocket_type::player_table && c->owner == target)
                        || c == target->get_character();
                });
                origin->m_game->queue_action([=]{
                    origin->m_game->remove_disablers(key);
                }, 90);
                origin->m_game->remove_listeners(key);
            }
        });
    }

    void modifier_bigfifty::add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx) {
        ctx.get<contexts::disable_bang_checks>() = true;
    }
}