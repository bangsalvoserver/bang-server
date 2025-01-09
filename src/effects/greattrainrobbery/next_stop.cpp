#include "next_stop.h"

#include "ruleset.h"

#include "game/game_table.h"

namespace banggame {

    void effect_next_stop::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->play_sound("train");
        
        origin->m_game->add_log("LOG_TRAIN_ADVANCE");
        origin->m_game->add_update<"move_train">(++origin->m_game->train_position);

        origin->m_game->call_event(event_type::on_train_advance{ origin, std::make_shared<locomotive_context>(1) });
    }
}