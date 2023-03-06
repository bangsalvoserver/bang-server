#ifndef __GAME_H__
#define __GAME_H__

#include "events.h"
#include "game_table.h"
#include "request_queue.h"
#include "draw_check_handler.h"
#include "player_iterator.h"
#include "utils/utils.h"
#include "utils/generator.h"

namespace banggame {

    struct game : game_table, listener_map, request_queue {
        draw_check_handler m_current_check;

        player *m_playing = nullptr;

        game() : request_queue(this), m_current_check(this) {}

        util::generator<json::json> get_spectator_updates();
        util::generator<json::json> get_rejoin_updates(player *target);

        card_sign get_card_sign(card *c);

        void add_players(std::span<int> user_ids);
        void start_game(const game_options &options);

        void apply_rulesets();

        bool request_bot_play(player *origin, bool is_response);

        request_status_args make_request_update(player *p);
        status_ready_args make_status_ready_update(player *p);

        void send_request_status_clear();
        void send_request_status_ready();
        void send_request_update();

        void start_next_turn();

        void draw_check_then(player *origin, card *origin_card, draw_check_condition condition, draw_check_function fun);

        void handle_player_death(player *killer, player *target, discard_all_reason reason);
    };

}

#endif