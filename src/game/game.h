#ifndef __GAME_H__
#define __GAME_H__

#include "game_table.h"

#include "net/game_interface.h"

namespace banggame {

    struct game : game_interface, game_table {
        using game_table::game_table;
    
        void tick() override {
            game_table::tick();
        }
        
        void get_pending_updates(std::span<const int> user_ids, consumer_callback<int, update_content> callback) override;
        void get_spectator_join_updates(consumer_callback<update_content> callback) override;
        void get_rejoin_updates(int user_id, consumer_callback<update_content> callback) override;

        void handle_game_action(int user_id, const json::json &action) override;

        void rejoin_user(int old_user_id, int new_user_id) override;
        void start_game(std::span<int> user_ids) override;

        bool is_game_over() const override {
            return game_table::is_game_over();
        }

        void get_game_commands(bool enable_cheats, consumer_callback<chat_command> callback) const override;

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