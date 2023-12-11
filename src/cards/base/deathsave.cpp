#include "deathsave.h"

#include "game/game.h"

#include "cards/game_enums.h"

namespace banggame {

    bool effect_deathsave::can_play(card *origin_card, player *origin) {
        return origin->m_game->top_request<request_death>(origin) != nullptr;
    }

    void effect_deathsave::on_play(card *origin_card, player *origin) {
        origin->m_game->top_request<request_death>()->tried_save = true;
    }

    void request_death::on_update() {
        if (target->m_hp <= 0 && !target->check_player_flags(player_flags::dead)) {
            auto_respond();
        } else {
            target->m_game->pop_request();
        }
    }
    
    void request_death::on_resolve() {
        target->m_game->pop_request();
        target->m_game->call_event(event_type::on_player_death_resolve{ target, tried_save });
        target->m_game->handle_player_death(origin, target, discard_all_reason::death);
    }

    game_string request_death::status_text(player *owner) const {
        int nbeers = 1 - target->m_hp;
        if (target == owner) {
            return {"STATUS_DEATH", nbeers};
        } else {
            return {"STATUS_DEATH_OTHER", target, nbeers};
        }
    }
}