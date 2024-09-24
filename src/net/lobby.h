#ifndef __LOBBY_H__
#define __LOBBY_H__

#include "options.h"
#include "messages.h"

#include "game/game.h"

namespace banggame {

class game_manager;
struct client_state;
struct game_user;
struct lobby;

using client_handle = std::weak_ptr<void>;

struct lobby_error : std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct critical_error : std::runtime_error {
    using std::runtime_error::runtime_error;
};

static constexpr ticks lobby_lifetime = 5min;
static constexpr ticks user_lifetime = 10s;

static constexpr ticks client_accept_timer = 5s;
static constexpr ticks ping_interval = 10s;
static constexpr auto pings_until_disconnect = 2min / ping_interval;

struct client_state {
    client_state(client_handle client)
        : client{client}
        , state{not_validated{}} {}
    
    client_handle client;

    struct not_validated {
        ticks timeout = ticks{0};
    };
    struct connected {
        game_user *user = nullptr;
        ticks ping_timer = ticks{0};
        int ping_count = 0;
    };
    struct invalid {};
    
    std::variant<not_validated, connected, invalid> state;
};

struct game_user {
    explicit game_user(id_type session_id): session_id{session_id} {}

    std::string username;
    utils::image_pixels propic;
    
    id_type session_id = 0;
    lobby *in_lobby = nullptr;

    client_handle client;
    ticks lifetime = user_lifetime;

    std::chrono::milliseconds get_disconnect_lifetime() const;

    void set_username(const std::string &new_username);
    void set_propic(const utils::image_pixels &new_propic);
};

enum class lobby_user_flag {
    muted
};

struct lobby_user {
    lobby_team team;
    int user_id;
    game_user *user;
    enums::bitset<lobby_user_flag> flags;
};

struct lobby_bot {
    int user_id;
    std::string username;
    const utils::image_pixels *propic;
};

struct lobby : lobby_info {
    lobby(const lobby_info &info, id_type lobby_id)
        : lobby_info{info}, lobby_id{lobby_id} {}

    id_type lobby_id;
    int user_id_count = 0;

    std::vector<lobby_user> users;
    std::vector<lobby_bot> bots;
    std::vector<lobby_chat_args> chat_messages;
    
    lobby_state state;
    ticks lifetime = lobby_lifetime;

    std::unique_ptr<banggame::game> m_game;

    lobby_user &add_user(game_user &user);

    lobby_user &find_user(const game_user &user);
    lobby_user &find_user(std::string_view name_or_id);

    explicit operator lobby_data() const;
};

using user_map = std::unordered_map<id_type, game_user>;
using client_map = std::map<client_handle, client_state, std::owner_less<>>;
using lobby_map = std::unordered_map<id_type, lobby>;
using lobby_list = std::vector<lobby_map::iterator>;

}

#endif