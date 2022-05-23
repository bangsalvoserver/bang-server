#ifndef __GAME_H__
#define __GAME_H__

#include "events.h"
#include "game_table.h"
#include "request_queue.h"
#include "draw_check_handler.h"
#include "player_iterator.h"

namespace banggame {

    struct game : game_table, event_handler_map, request_queue<game> {
        bool m_game_over = false;

        draw_check_handler m_current_check;

        player *m_playing = nullptr;

        player *find_disconnected_player();

        std::vector<game_update> get_game_state_updates(player *owner);

        void start_game(const game_options &options);

        request_status_args make_request_update(player *p);
        void send_request_update();

        void start_next_turn();

        void draw_check_then(player *origin, card *origin_card, draw_check_function fun);

        void check_game_over(player *killer, player *target);
        void player_death(player *killer, player *target);
    };

}

#endif