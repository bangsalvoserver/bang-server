#ifndef __GAME_H__
#define __GAME_H__

#include "game_table.h"
#include "request_queue.h"
#include "player_iterator.h"
#include "utils/utils.h"
#include "utils/generator.h"

namespace banggame {

    struct game : game_table, request_queue {
        game(unsigned int seed = 0);
        
        util::generator<json::json> get_spectator_join_updates();
        util::generator<json::json> get_game_log_updates(player *target);
        util::generator<json::json> get_rejoin_updates(player *target);

        card_sign get_card_sign(card *c);

        void add_players(std::span<int> user_ids);
        void start_game(const game_options &options);

        player_distances make_player_distances(player *p);
        request_status_args make_request_update(player *p);
        status_ready_args make_status_ready_update(player *p);
        player_order_update make_player_order_update(bool instant = false);

        void send_request_status_clear();
        bool send_request_status_ready();
        void send_request_update();
        
        void draw_scenario_card();
        void advance_train(player *origin);

        void start_next_turn();

        bool request_bot_play();

        void draw_check_then(player *origin, card *origin_card, draw_check_condition condition, draw_check_function fun);

        void handle_player_death(player *killer, player *target, discard_all_reason reason);
    };

}

#endif