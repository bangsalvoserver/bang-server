#include "lobby.h"
#include "bot_info.h"

#include "utils/range_utils.h"

namespace banggame {

    void game_session::set_username(const std::string &new_username) {
        static constexpr size_t max_username_size = 50;

        if (new_username.size() > max_username_size) {
            username = new_username.substr(0, max_username_size);
        } else {
            username = new_username;
        }
    }

    static utils::image_pixels scale_image(const utils::image_pixels &image, int new_size) {
        int width = image.width;
        int height = image.height;

        if (width <= 0 || height <= 0 || new_size <= 0) {
            return utils::image_pixels{};
        }

        if (width > height) {
            height = new_size * height / width;
            width = new_size;
        } else {
            width = new_size * width / height;
            height = new_size;
        }

        if (width == image.width && height == image.height) {
            return image;
        }

        utils::image_pixels result { width, height };
        result.pixels.resize(width * height * 4);
        for (size_t y = 0; y < height; ++y) {
            for (size_t x = 0; x < width; ++x) {
                size_t scaled_x = x * image.width / width;
                size_t scaled_y = y * image.height / height;

                result.set_pixel(x, y, image.get_pixel(scaled_x, scaled_y));
            }
        }
        return result;
    }

    void game_session::set_propic(const utils::image_pixels &new_propic) {
        propic = scale_image(new_propic, bot_info.propic_size);
    }

    static auto find_user_it(auto &list, const game_session &session) {
        return rn::find(list, &session, [](const game_user &user) { return &user.session; });
    }

    std::pair<game_user &, bool> game_lobby::add_user(game_session &session) {
        if (auto it = find_user_it(users, session); it != users.end()) {
            return {*it, false};
        } else {
            session.lobby = this;
            return {users.emplace_back(++user_id_count, session), true};
        }
    }

    game_user game_lobby::remove_user(const game_session &session) {
        auto it = find_user_it(users, session);
        game_user user = std::move(*it);
        users.erase(it);
        return user;
    }

    game_user &game_lobby::find_user(const game_session &session) {
        if (auto it = find_user_it(users, session); it != users.end()) {
            return *it;
        }
        throw lobby_error("CANNOT_FIND_USER");
    }

    game_user &game_lobby::find_user(std::string_view name_or_id) {
        int user_id;
        if (auto [end, ec] = std::from_chars(name_or_id.data(), name_or_id.data() + name_or_id.size(), user_id); ec == std::errc{}) {
            if (auto it = rn::find(users, user_id, &game_user::user_id); it != users.end()) {
                return *it;
            }
        }

        if (game_user *user = get_single_element(users
            | rv::filter([&](const game_user &user) {
                return string_equal_icase(user.session.username, name_or_id);
            })
            | rv::addressof))
        {
            return *user;
        }
        
        throw lobby_error("CANNOT_FIND_USER");
    }

    void game_lobby::update_lobby_info(const lobby_info &info) {
        static constexpr size_t max_lobby_name_size = 50;

        if (info.name.size() > max_lobby_name_size) {
            name = info.name.substr(0, max_lobby_name_size);
        } else {
            name = info.name;
        }

        options = info.options;   
    }

    game_lobby::operator lobby_data() const {
        return {
            .lobby_id = lobby_id,
            .name = name,
            .num_players = int(rn::count(users, lobby_team::game_player, &game_user::team)),
            .num_spectators = int(rn::count(users, lobby_team::game_spectator, &game_user::team)),
            .max_players = lobby_max_players,
            .secure = !password.empty(),
            .state = state
        };
    }
}