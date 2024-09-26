#include "lobby.h"

#include "utils/range_utils.h"

namespace banggame {

    std::chrono::milliseconds game_user::get_disconnect_lifetime() const {
        if (client.expired()) {
            return std::chrono::duration_cast<std::chrono::milliseconds>(lifetime);
        }
        return {};
    }

    void game_user::set_username(const std::string &new_username) {
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

    void game_user::set_propic(const utils::image_pixels &new_propic) {
        static constexpr int propic_size = 200;

        propic = scale_image(new_propic, propic_size);
    }

    lobby_user &lobby::add_user(game_user &user) {
        if (auto it = rn::find(users, &user, &lobby_user::user); it != users.end()) {
            return *it;
        } else {
            user.in_lobby = this;
            return users.emplace_back(lobby_team::game_player, ++user_id_count, &user);
        }
    }

    lobby_user &lobby::find_user(const game_user &user) {
        if (auto it = rn::find(users, &user, &lobby_user::user); it != users.end()) {
            return *it;
        }
        throw lobby_error("CANNOT_FIND_USER");
    }

    lobby_user &lobby::find_user(std::string_view name_or_id) {
        int user_id;
        if (auto [end, ec] = std::from_chars(name_or_id.data(), name_or_id.data() + name_or_id.size(), user_id); ec == std::errc{}) {
            if (auto it = rn::find(users, user_id, &lobby_user::user_id); it != users.end()) {
                return *it;
            }
        }

        if (lobby_user *lu = get_single_element(users
            | rv::filter([&](const lobby_user &user) {
                return string_equal_icase(user.user->username, name_or_id);
            })
            | rv::addressof))
        {
            return *lu;
        }
        
        throw lobby_error("CANNOT_FIND_USER");
    }

    void lobby::update_lobby_info(const lobby_info &info) {
        static constexpr size_t max_lobby_name_size = 50;

        if (info.name.size() > max_lobby_name_size) {
            name = info.name.substr(0, max_lobby_name_size);
        } else {
            name = info.name;
        }

        options = info.options;   
    }

    lobby::operator lobby_data() const {
        return {
            .lobby_id = lobby_id,
            .name = name,
            .num_players = int(rn::count(users, lobby_team::game_player, &lobby_user::team)),
            .num_spectators = int(rn::count(users, lobby_team::game_spectator, &lobby_user::team)),
            .max_players = lobby_max_players,
            .state = state
        };
    }
}