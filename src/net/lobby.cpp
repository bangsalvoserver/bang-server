#include "lobby.h"
#include "bot_info.h"

#include "utils/range_utils.h"

namespace banggame {

    void game_session::set_username(std::string new_username) {
        static constexpr size_t max_username_size = 50;

        if (new_username.size() > max_username_size) {
            username = new_username.substr(0, max_username_size);
        } else {
            username = std::move(new_username);
        }
    }

    void game_session::set_propic(image_pixels new_propic) {
        propic.reset(std::move(new_propic).scale_to(bot_info.propic_size));
    }

    server_messages::lobby_user_update game_user::make_user_update() const {
        return {
            .user_id = user_id,
            .username = session->username,
            .propic = session->propic,
            .flags = flags,
            .lifetime = (!is_disconnected() && session->client.expired())
                ? std::chrono::duration_cast<std::chrono::milliseconds>(session->lifetime)
                : 0ms
        };
    }

    server_messages::lobby_user_update lobby_bot::make_user_update() const {
        return {
            .user_id = user_id,
            .username = username,
            .propic = propic
        };
    }

    game_user &game_lobby::find_user(int user_id) {
        return users[user_id - 1];
    }

    game_user &game_lobby::find_user(session_ptr session) {
        auto it = users_by_session.find(session);
        if (it != users_by_session.end()) {
            return find_user(it->second);
        }
        throw lobby_error("CANNOT_FIND_USER");
    }

    game_user &game_lobby::find_user(std::string_view name_or_id) {
        auto range = connected_users();

        int user_id;
        if (auto [end, ec] = std::from_chars(name_or_id.data(), name_or_id.data() + name_or_id.size(), user_id); ec == std::errc{}) {
            if (user_id >= 1 && user_id <= users.size()) {
                game_user &user = find_user(user_id);
                if (!user.is_disconnected()) {
                    return user;
                }
            }
        }

        if (game_user *user = get_single_element(range
            | rv::filter([&](const game_user &user) {
                return string_equal_icase(user.session->username, name_or_id);
            })
            | rv::addressof))
        {
            return *user;
        }
        
        throw lobby_error("CANNOT_FIND_USER");
    }
    
    std::pair<game_user &, bool> game_lobby::add_user(session_ptr session) {
        session->lobby = this;
        if (auto it = users_by_session.find(session); it != users_by_session.end()) {
            return {find_user(it->second), false};
        } else {
            int user_id = users.size() + 1;
            game_user &user = users.emplace_back(user_id, session);
            users_by_session.emplace(session, user_id);
            connected_user_ids.push_back(user_id);
            return {user, true};
        }
    }

    std::string game_lobby::crop_lobby_name(const std::string &name) {
        static constexpr size_t max_lobby_name_size = 50;

        if (name.size() > max_lobby_name_size) {
            return name.substr(0, max_lobby_name_size);
        } else {
            return name;
        }
    }

    server_messages::lobby_update game_lobby::make_lobby_update(session_ptr owner) const {
        return {
            .lobby_id = lobby_id,
            .name = name,
            .num_players = int(rn::count_if(connected_users(), std::not_fn(&game_user::is_spectator))),
            .num_bots = int(bots.size()),
            .num_spectators = int(rn::count_if(connected_users(), &game_user::is_spectator)),
            .security = password.empty()
                ? lobby_security::open
                : (owner != nullptr && rn::contains(users, owner, &game_user::session))
                    ? lobby_security::unlocked
                    : lobby_security::locked,
            .state = state
        };
    }
}