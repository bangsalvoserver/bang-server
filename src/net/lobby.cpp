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

    static auto find_user_it(auto &list, session_ptr session) {
        return rn::find(list, session, &game_user::session);
    }
    
    std::pair<game_user &, bool> game_lobby::add_user(session_ptr session) {
        session->lobby = this;
        if (auto it = find_user_it(users, session); it != users.end()) {
            return {*it, false};
        } else {
            return {users.emplace_back(++user_id_count, session), true};
        }
    }

    game_user &game_lobby::find_user(session_ptr session) {
        if (auto it = find_user_it(users, session); it != users.end()) {
            return *it;
        }
        throw lobby_error("CANNOT_FIND_USER");
    }

    game_user &game_lobby::find_user(std::string_view name_or_id) {
        auto range = connected_users();

        int user_id;
        if (auto [end, ec] = std::from_chars(name_or_id.data(), name_or_id.data() + name_or_id.size(), user_id); ec == std::errc{}) {
            if (auto it = rn::find(range, user_id, &game_user::user_id); it != range.end()) {
                return *it;
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

    std::string game_lobby::crop_lobby_name(const std::string &name) {
        static constexpr size_t max_lobby_name_size = 50;

        if (name.size() > max_lobby_name_size) {
            return name.substr(0, max_lobby_name_size);
        } else {
            return name;
        }
    }

    game_lobby::operator lobby_data() const {
        return {
            .lobby_id = lobby_id,
            .name = name,
            .num_players = int(rn::count_if(connected_users(), std::not_fn(&game_user::is_spectator))),
            .num_spectators = int(rn::count_if(connected_users(), &game_user::is_spectator)),
            .max_players = lobby_max_players,
            .secure = !password.empty(),
            .state = state
        };
    }
}