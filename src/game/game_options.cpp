#include "game_options.h"

#include "cards/expansion_set.h"
#include "utils/static_map.h"

namespace banggame {

    template<typename T, std::convertible_to<T> U>
    void clamp_value(T &value, const U &min, const U &max) {
        value = std::clamp<T>(value, min, max);
    }

    void validate_game_options(game_options &options) {
        if (!validate_expansions(options.expansions)) {
            options.expansions = {};
        }
        clamp_value(options.character_choice, 1, 3);
        clamp_value(options.auto_resolve_timer, 0s, 5s);
        clamp_value(options.damage_timer, 0s, 5s);
        clamp_value(options.escape_timer, 0s, 10s);
        clamp_value(options.bot_play_timer, 0s, 10s);
        clamp_value(options.duration_coefficient, 0.f, 4.f);
    }

    std::string game_options::to_string(std::string_view sep) const {
        std::string result;
        reflect::for_each<game_options>([&](auto I) {
            if constexpr (I != 0) {
                result += sep;
            }
            result += std::format("{} = {}", reflect::member_name<I>(*this), reflect::get<I>(*this));
        });
        return result;
    }

    void game_options::set_option(std::string_view key, std::string_view value) {
        static constexpr auto set_option_map = []<size_t ... Is>(std::index_sequence<Is ...>){
            using set_option_fn_ptr = void (*)(game_options &options, std::string_view value_str);

            return utils::make_static_map<std::string_view, set_option_fn_ptr>({
                { reflect::member_name<Is, game_options>(), [](game_options &options, std::string_view value_str) {
                    auto &field = reflect::get<Is>(options);
                    if (auto value = utils::parse_string<std::remove_reference_t<decltype(field)>>(value_str)) {
                        field = *value;
                    } else {
                        throw std::runtime_error("INVALID_OPTION_VALUE");
                    }
                }} ... });
        }(std::make_index_sequence<reflect::size<game_options>()>());
        
        auto it = set_option_map.find(key);
        if (it == set_option_map.end()) {
            throw std::runtime_error("INVALID_OPTION_NAME");
        }
        
        it->second(*this, value);
        validate_game_options(*this);
    }
    
    game_options game_options::deserialize_json(const json::json &value) {
        game_options result{};
        if (value.is_object()) {
            reflect::for_each<game_options>([&](auto I) {
                auto member_name = reflect::member_name<I, game_options>();
                if (auto it = value.find(member_name); it != value.end()) {
                    try {
                        auto &field = reflect::get<I>(result);

                        using option_type = reflect::member_type<I, game_options>;
                        field = json::deserialize<option_type>(*it);
                    } catch (const std::exception &error) {
                        // ignore errors.
                        // game_options are stored in the clients' application storage and we don't want them kicked out if it's invalid.
                    }
                }
            });
        }
        validate_game_options(result);
        return result;
    }

}