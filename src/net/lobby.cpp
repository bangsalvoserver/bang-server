#include "lobby.h"
#include "bot_info.h"
#include "manager.h"

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
            game_user &user = find_user(it->second);
            if (user.is_disconnected()) {
                user.flags.remove(game_user_flag::disconnected);
                connected_user_ids.push_back(user.user_id);
                return {user, true};
            }
            return {user, false};
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
            .security = get_security(owner),
            .state = state
        };
    }

    lobby_security game_lobby::get_security(session_ptr owner) const {
        if (password.empty()) {
            return lobby_security::open;
        } else if (owner != nullptr && rn::contains(users, owner, &game_user::session)) {
            return lobby_security::unlocked;
        } else {
            return lobby_security::locked;
        }
    }
    
    bool game_lobby::add_user_flag(game_user &user, game_user_flag flag) {
        if (!user.flags.check(flag)) {
            user.flags.add(flag);
            m_mgr->broadcast_message_lobby(*this, user.make_user_update());
            return true;
        }
        return false;
    }

    bool game_lobby::remove_user_flag(game_user &user, game_user_flag flag) {
        if (user.flags.check(flag)) {
            user.flags.remove(flag);
            m_mgr->broadcast_message_lobby(*this, user.make_user_update());
            return true;
        }
        return false;
    }

    void game_lobby::add_chat_message(server_messages::lobby_chat message, game_user *is_read_for) {
        server_messages::lobby_chat with_is_read = message;
        with_is_read.flags.add(lobby_chat_flag::is_read);
        for (const game_user &user : connected_users()) {
            if (&user == is_read_for) {
                m_mgr->send_message(user.session->client, with_is_read);
            } else {
                m_mgr->send_message(user.session->client, message);
            }
        }
        chat_messages.emplace_back(std::move(with_is_read));
    }

    void game_lobby::send_chat_message(int user_id, server_messages::lobby_chat message) {
        m_mgr->send_message(find_user(user_id).session->client, std::move(message));
    }
}