#include "deathsave.h"

#include "game/game.h"

namespace banggame {

    bool effect_deathsave::can_respond(card *origin_card, player *origin) {
        return origin->m_game->top_request_if<request_death>(origin);
    }

    void effect_deathsave::on_play(card *origin_card, player *origin) {
        auto lock = origin->m_game->lock_updates();
        if (origin->m_hp > 0) {
            origin->m_game->pop_request();
        } else {
            origin->m_game->top_request().get<request_death>().tried_save = true;
        }
    }
    
    void request_death::on_resolve() {
        auto lock = target->m_game->lock_updates(true);
        
        target->m_game->queue_action([target=target, tried_save=tried_save]{
            target->m_game->call_event<event_type::on_player_death_resolve>(target, tried_save);
        }, 2);
        
        target->m_game->queue_action([origin=origin, target=target]{
            if (target->m_hp <= 0) {
                target->m_game->handle_player_death(origin, target, discard_all_reason::death);
            }
        }, 2);
    }

    game_string request_death::status_text(player *owner) const {
        card *saving_card = [this]() -> card * {
            for (card *c : target->m_characters) {
                if (can_respond(target, c)) return c;
            }
            for (card *c : target->m_table) {
                if (can_respond(target, c)) return c;
            }
            for (card *c : target->m_hand) {
                if (can_respond(target, c)) return c;
            }
            return nullptr;
        }();
        if (saving_card) {
            if (target == owner) {
                return {"STATUS_DEATH", saving_card};
            } else {
                return {"STATUS_DEATH_OTHER", target, saving_card};
            }
        } else {
            return "STATUS_DEATH";
        }
    }
}