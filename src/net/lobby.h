#ifndef __LOBBY_H__
#define __LOBBY_H__

#include "options.h"
#include "messages.h"
#include "image_registry.h"

#include "game/game.h"

namespace banggame {

class game_manager;
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

struct game_session {
    std::string username;
    image_registry::registered_image propic;
    
    game_lobby *lobby = nullptr;

    client_handle client;
    ticks lifetime = user_lifetime;

    void set_username(std::string new_username);
    void set_propic(image_pixels new_propic);
};

using session_ptr = std::shared_ptr<game_session>;

namespace connection_state {
    struct not_validated {
        ticks timeout = ticks{0};
    };
    
    struct connected {
        connected(session_ptr session): session{session} {}

        session_ptr session;
        ticks ping_timer = ticks{0};
        int ping_count = 0;
    };

    struct invalid {};
}

using connection = std::variant<
    connection_state::not_validated,
    connection_state::connected,
    connection_state::invalid
>;

struct game_user {
    game_user(int user_id, session_ptr session)
        : user_id{user_id}, session{session} {}
    
    int user_id;
    session_ptr session;
    game_user_flags flags;

    bool is_disconnected() const {
        return flags.check(game_user_flag::disconnected);
    }

    bool is_spectator() const {
        return flags.check(game_user_flag::spectator);
    }

    bool is_muted() const {
        return flags.check(game_user_flag::muted);
    }
};

struct lobby_bot {
    int user_id;
    std::string username;
    image_pixels_hash propic;
};

struct game_lobby {
    game_lobby(id_type lobby_id, const std::string &name, const game_options &options)
        : lobby_id{lobby_id}
        , name{crop_lobby_name(name)}
        , options{options} {}

    id_type lobby_id;
    int user_id_count = 0;

    std::string name;
    game_options options;

    std::string password;

    std::vector<game_user> users;
    std::vector<lobby_bot> bots;
    std::vector<lobby_chat_args> chat_messages;
    
    lobby_state state;
    ticks lifetime = lobby_lifetime;

    std::unique_ptr<banggame::game> m_game;

    auto connected_users(this auto &&self) {
        return rv::remove_if(std::forward_like<decltype(self)>(self.users), &game_user::is_disconnected);
    }

    bool is_owner(session_ptr session) const {
        if (auto range = connected_users()) {
            return range.front().session == session;
        }
        return false;
    }

    std::pair<game_user &, bool> add_user(session_ptr session);

    game_user &find_user(session_ptr session);
    game_user &find_user(std::string_view name_or_id);

    static std::string crop_lobby_name(const std::string &name);

    explicit operator lobby_data() const;
};

using user_map = std::unordered_map<id_type, session_ptr>;
using client_map = std::map<client_handle, connection, std::owner_less<>>;
using lobby_map = std::map<id_type, game_lobby>;

}

#endif