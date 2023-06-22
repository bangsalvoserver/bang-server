#ifndef __LOBBY_H__
#define __LOBBY_H__

#include "options.h"
#include "messages.h"

#include "game/game.h"

#include <list>

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

using lobby_list = std::list<lobby>;
using lobby_ptr = lobby_list::iterator;

struct game_user: user_info {
    game_user(int user_id, const user_info &info)
        : user_info{info}, user_id{user_id} {}
    
    int user_id = 0;
    lobby *in_lobby = nullptr;
};

struct lobby : lobby_info {
    lobby(int id, const lobby_info &info)
        : lobby_info{info}, id{id} {}

    int id;

    std::vector<team_user_pair> users;
    std::vector<game_user> bots;
    std::vector<lobby_chat_args> chat_messages;
    
    lobby_state state;
    ticks lifetime = lobby_lifetime;

    std::unique_ptr<banggame::game> m_game;
    void start_game(game_manager &mgr);
    void send_updates(game_manager &mgr);
    lobby_data make_lobby_data() const;
};

}

#endif