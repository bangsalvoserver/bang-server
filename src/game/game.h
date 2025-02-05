#ifndef __GAME_H__
#define __GAME_H__

#include "game_table.h"

#include "utils/function_ref.h"

namespace banggame {

    using send_update_function = std23::function_ref<void (json::json &&) const>;

    struct game : game_table {
        using game_table::game_table;
        
        void send_spectator_join_updates(const send_update_function &send_update);
        void send_game_log_updates(player_ptr target, const send_update_function &send_update);
        void send_rejoin_updates(player_ptr target, const send_update_function &send_update);

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