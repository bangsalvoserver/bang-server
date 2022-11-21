#ifndef __LOBBY_H__
#define __LOBBY_H__

#include <json/json.h>

#include "options.h"
#include "messages.h"

#include "game/game.h"
#include "game/durations.h"

namespace banggame {

class game_manager;
struct game_user;

using client_handle = std::weak_ptr<void>;
using user_map = std::map<client_handle, game_user, std::owner_less<client_handle>>;
using user_ptr = user_map::iterator;

DEFINE_ENUM(lobby_team,
    (game_player)
    (game_spectator)
)

using team_user_pair = std::pair<lobby_team, user_ptr>;

struct lobby : lobby_info {
    std::vector<team_user_pair> users;
    lobby_state state;
    ticks lifetime = lobby_lifetime;

    banggame::game game;
    void start_game(game_manager &mgr);
    void send_updates(game_manager &mgr);
};

using lobby_map = std::map<int, lobby>;
using lobby_ptr = lobby_map::iterator;

struct game_user {
    int user_id;
    std::string name;
    sdl::image_pixels profile_image;
    std::optional<lobby_ptr> in_lobby;
};

}

#endif