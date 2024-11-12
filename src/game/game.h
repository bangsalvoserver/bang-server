#ifndef __GAME_H__
#define __GAME_H__

#include "game_table.h"

#include <generator>

namespace banggame {

    struct game : game_table {
        using game_table::game_table;
        
        std::generator<json::json> get_spectator_join_updates();
        std::generator<json::json> get_game_log_updates(player_ptr target);
        std::generator<json::json> get_rejoin_updates(player_ptr target);

        card_ptr add_card(const card_data &data);
        void add_players(std::span<int> user_ids);
        void start_game();

        player_distances make_player_distances(player_ptr p);
        request_status_args make_request_update(player_ptr p);
        status_ready_args make_status_ready_update(player_ptr p);
        player_order_update make_player_order_update(bool instant = false);

        ticks get_total_update_time() const override;
        void send_request_update() override;
        void send_request_status_clear() override;
        request_state send_request_status_ready() override;
        request_state request_bot_play(bool instant) override;

        void start_next_turn();

        void handle_player_death(player_ptr killer, player_ptr target, discard_all_reason reason);
    };

}

#endif