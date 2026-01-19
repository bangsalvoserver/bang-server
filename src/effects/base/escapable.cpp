#include "escapable.h"

#include "game/game_table.h"
#include "game/game_options.h"

namespace banggame {

    escape_type request_escapable_resolvable::get_escape_type() const {
        return target->m_game->call_event(event_type::apply_escapable_modifier{ origin_card, origin, target, flags, *this });
    }

    void request_targeting::on_update() {
        if (target->immune_to(origin_card, origin, flags)) {
            target->m_game->pop_request();
        } else if (origin == target_card->owner && auto_resolvable()) {
            // auto resolve if targeting self, unless player has escape
            on_resolve();
        } else {
            switch (get_escape_type()) {
            case escape_type::no_escape:
                auto_resolve();
                break;
            case escape_type::escape_timer:
                set_duration(target->m_game->m_options.escape_timer);
                break;
            case escape_type::escape_no_timer:
                // ignore
                break;
            }
        }
    }
    
    card_list request_targeting::get_highlights(player_ptr owner) const {
        if (target_card->pocket == pocket_type::player_hand) {
            return target->m_hand;
        } else {
            return {target_card};
        }
    }

}