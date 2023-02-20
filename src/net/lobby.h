#ifndef __LOBBY_H__
#define __LOBBY_H__

#include "options.h"
#include "messages.h"

#include "game/game.h"

namespace banggame {

class game_manager;
struct game_user;
struct lobby;

using client_handle = std::weak_ptr<void>;
using user_map = std::map<client_handle, game_user, std::owner_less<client_handle>>;
using user_ptr = user_map::iterator;

DEFINE_ENUM(lobby_team,
    (game_player)
    (game_spectator)
)

using team_user_pair = std::pair<lobby_team, user_ptr>;

static constexpr ticks lobby_lifetime = 10s;

using lobby_map = std::map<int, lobby>;
using lobby_ptr = lobby_map::iterator;

struct game_user : lobby_add_user_args {
    game_user(auto && ... args)
        : lobby_add_user_args(FWD(args) ... ) {}
    
    std::optional<lobby_ptr> in_lobby;
};

struct lobby : lobby_info {
    std::vector<team_user_pair> users;
    std::vector<game_user> bots;
    lobby_state state;
    ticks lifetime = lobby_lifetime;

    std::unique_ptr<banggame::game> m_game;
    void start_game(game_manager &mgr);
    void send_updates(game_manager &mgr);
};

}

#endif