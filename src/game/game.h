#ifndef __GAME_H__
#define __GAME_H__

#include "game_table.h"

#include <generator>

namespace banggame {

    struct game : game_table {
        using game_table::game_table;
        
        std::generator<game_update> get_spectator_join_updates();
        std::generator<game_update> get_game_log_updates(player_ptr target);
        std::generator<game_update> get_rejoin_updates(player_ptr target);

        void start_game();

        ticks get_total_update_time() const override;
        void send_request_update() override;
        void send_request_status_clear() override {
            clear_request_status();
        }
        request_state send_request_status_ready() override;
        request_state request_bot_play(bool instant) override;
    };

}

#endif