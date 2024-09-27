#ifndef __LOBBY_H__
#define __LOBBY_H__

#include "options.h"
#include "messages.h"

#include "game/game.h"

#include <list>

namespace banggame {

class game_manager;
struct connection;
struct game_session;
struct game_lobby;

using client_handle = std::weak_ptr<void>;

struct lobby_error : std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct critical_error : std::runtime_error {
    using std::runtime_error::runtime_error;
};

static constexpr ticks lobby_lifetime = 5min;
static constexpr ticks user_lifetime = 10s;

static constexpr ticks client_accept_timer = 30s;
static constexpr ticks ping_interval = 10s;
static constexpr auto pings_until_disconnect = 2min / ping_interval;

struct connection {
    connection(client_handle client) : client{client} {}
    
    client_handle client;

    struct not_validated {
        ticks timeout = ticks{0};
    };
    struct connected {
        connected(game_session &session): session{session} {}

        game_session &session;
        ticks ping_timer = ticks{0};
        int ping_count = 0;
    };
    struct invalid {};
    
    std::variant<not_validated, connected, invalid> state = not_validated{};
};

struct game_session {
    std::string username;
    utils::image_pixels propic;
    
    game_lobby *lobby = nullptr;

    client_handle client;
    ticks lifetime = user_lifetime;

    std::chrono::milliseconds get_disconnect_lifetime() const;

    void set_username(const std::string &new_username);
    void set_propic(const utils::image_pixels &new_propic);
};

enum class lobby_user_flag {
    muted
};

struct game_user {
    game_user(int user_id, game_session &session)
        : user_id{user_id}, session{session} {}
    
    int user_id;
    game_session &session;
    lobby_team team = lobby_team::game_player;
    enums::bitset<lobby_user_flag> flags;
};

struct lobby_bot {
    int user_id;
    std::string username;
    const utils::image_pixels *propic;
};

struct game_lobby : lobby_info {
    game_lobby(const lobby_info &info, id_type lobby_id)
        : lobby_id{lobby_id}
    {
        update_lobby_info(info);
    }

    id_type lobby_id;
    int user_id_count = 0;

    std::list<game_user> users;
    std::vector<lobby_bot> bots;
    std::vector<lobby_chat_args> chat_messages;
    
    lobby_state state;
    ticks lifetime = lobby_lifetime;

    std::unique_ptr<banggame::game> m_game;

    game_user &add_user(game_session &session);
    game_user remove_user(const game_session &session);

    game_user &find_user(const game_session &session);
    game_user &find_user(std::string_view name_or_id);

    void update_lobby_info(const lobby_info &info);

    explicit operator lobby_data() const;
};

using user_map = std::unordered_map<id_type, game_session>;
using client_map = std::map<client_handle, connection, std::owner_less<>>;
using lobby_map = std::map<id_type, game_lobby>;

}

#endif