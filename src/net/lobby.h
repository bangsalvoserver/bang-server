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

static constexpr ticks lobby_lifetime = 5min;
static constexpr ticks ping_interval = 10s;
static constexpr int pings_until_disconnect = 10;

using lobby_list = std::list<lobby>;
using lobby_ptr = lobby_list::iterator;

struct game_user: user_info {
    game_user(const user_info &info, int session_id)
        : user_info{info}, session_id{session_id} {}
    
    int session_id = 0;
    lobby *in_lobby = nullptr;

    ticks ping_timer{};
    int ping_count = 0;
};

struct lobby_user {
    lobby_team team;
    int user_id;
    user_ptr user;
};

struct session_id_to_index {
    int session_id;
    int user_id;
};

struct lobby : lobby_info {
    lobby(int id, const lobby_info &info)
        : lobby_info{info}, id{id} {}

    int id;
    int user_id_count = 0;

    std::vector<lobby_user> users;
    std::vector<game_user> bots;
    std::vector<lobby_chat_args> chat_messages;
    std::vector<session_id_to_index> disconnected_users;

    int get_user_id(user_ptr user) const {
        if (auto it = rn::find(users, user, &lobby_user::user); it != users.end()) {
            return it->user_id;
        }
        return 0;
    }
    
    lobby_state state;
    ticks lifetime = lobby_lifetime;

    std::unique_ptr<banggame::game> m_game;
    void start_game(game_manager &mgr);
    void send_updates(game_manager &mgr);
    lobby_data make_lobby_data() const;
};

}

#endif