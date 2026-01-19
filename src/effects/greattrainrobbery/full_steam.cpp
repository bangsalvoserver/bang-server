#include "full_steam.h"

#include "cards/game_enums.h"

#include "game/game_table.h"

#include "ruleset.h"

namespace banggame {

    prompt_string effect_full_steam::on_prompt(card_ptr origin_card, player_ptr origin) {
        return origin->m_game->call_event(event_type::get_locomotive_prompt{ origin, value });
    }
    
    void effect_full_steam::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->play_sound(sound_id::train);

        origin->m_game->add_log("LOG_TRAIN_ADVANCE");
        origin->m_game->train_position = int8_t(origin->m_game->m_stations.size());
        origin->m_game->add_update(game_updates::move_train{ origin->m_game->train_position });

        origin->m_game->call_event(event_type::on_train_advance{ origin, std::make_shared<locomotive_context>(value) });
    }
}