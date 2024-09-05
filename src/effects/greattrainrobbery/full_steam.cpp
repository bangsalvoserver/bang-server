#include "full_steam.h"

#include "game/game.h"
#include "ruleset.h"

namespace banggame {
    
    void effect_full_steam::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->play_sound("train");

        origin->m_game->add_log("LOG_TRAIN_ADVANCE");
        origin->m_game->train_position = int8_t(origin->m_game->m_stations.size());
        origin->m_game->add_update<"move_train">(origin->m_game->train_position);

        origin->m_game->call_event(event_type::on_train_advance{ origin, std::make_shared<locomotive_context>(value) });
    }
}