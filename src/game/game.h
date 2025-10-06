#ifndef __GAME_H__
#define __GAME_H__

#include "game_table.h"

#include <generator>

namespace banggame {

    struct game_interface {
        virtual ~game_interface() = default;

        virtual void tick() = 0;
        virtual bool pending_updates() const = 0;
        virtual std::generator<std::pair<int, json::raw_string>> get_pending_updates(std::span<const int> user_ids) = 0;
        virtual std::generator<json::raw_string> get_spectator_join_updates() = 0;
        virtual std::generator<json::raw_string> get_rejoin_updates(int user_id) = 0;
        virtual void handle_game_action(int user_id, const json::json &action) = 0;
        virtual void rejoin_user(int old_user_id, int new_user_id) = 0;
        virtual void start_game(std::span<int> user_ids) = 0;
        virtual bool is_game_over() const = 0;
    };

    struct game : game_interface, game_table {
        using game_table::game_table;
    
        void tick() override {
            game_table::tick();
        }

        bool pending_updates() const override {
            return !m_updates.empty();
        }
        
        std::generator<std::pair<int, json::raw_string>> get_pending_updates(std::span<const int> user_ids) override;
        std::generator<json::raw_string> get_spectator_join_updates() override;
        std::generator<json::raw_string> get_rejoin_updates(int user_id) override;

        void handle_game_action(int user_id, const json::json &action) override;

        void rejoin_user(int old_user_id, int new_user_id) override;
        void start_game(std::span<int> user_ids) override;

        bool is_game_over() const override {
            return game_table::is_game_over();
        }

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