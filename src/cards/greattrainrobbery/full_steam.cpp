#include "full_steam.h"

#include "game/game.h"
#include "cards/effect_context.h"

namespace banggame {
    
    void effect_full_steam::on_play(card *origin_card, player *origin) {
        origin->m_game->add_log("LOG_TRAIN_ADVANCE");
        origin->m_game->train_position = int8_t(origin->m_game->m_stations.size());
        origin->m_game->add_update<game_update_type::move_train>(origin->m_game->train_position);

        origin->m_game->call_event<event_type::on_train_advance>(origin,
            std::make_shared<effect_context_ptr>(new effect_context{ .locomotive_count = value }));
    }
}