#include "game_options.h"

#include "utils/static_map.h"

namespace banggame {

    constexpr size_t game_option_field_index(std::string_view name) {
        constexpr auto member_names = []<size_t ... Is>(std::index_sequence<Is ...>) {
            return std::array{
                reflect::member_name<Is, game_options>() ...
            };
        }(std::make_index_sequence<reflect::size<game_options>()>());
        auto it = rn::find(member_names, name);
        if (it != member_names.end()) {
            return rn::distance(member_names.begin(), it);
        }
        throw std::out_of_range("Cannot find game_option_field_index");
    }

    template<size_t I> constexpr auto game_option_transformer = [](const auto &value) { return value; };
    #define DEFINE_TRANSFORMER(FIELD, FUN) template<> constexpr auto game_option_transformer<game_option_field_index(#FIELD)> = FUN;

    DEFINE_TRANSFORMER(expansions, [](const expansion_set &value) {
        if (!validate_expansions(value)) {
            throw game_option_error("INVALID_EXPANSIONS");
        }
        return value;
    })

    template<typename T>
    constexpr auto clamp_value(const T &min, const T &max) {
        return [=](const auto &value) {
            return std::clamp<std::remove_cvref_t<decltype(value)>>(value, min, max);
        };
    }

    DEFINE_TRANSFORMER(character_choice, clamp_value(1, 3))
    DEFINE_TRANSFORMER(max_players, clamp_value(3, 8))
    DEFINE_TRANSFORMER(auto_resolve_timer, clamp_value(0s, 5s))
    DEFINE_TRANSFORMER(damage_timer, clamp_value(0s, 5s))
    DEFINE_TRANSFORMER(escape_timer, clamp_value(0s, 10s))
    DEFINE_TRANSFORMER(bot_play_timer, clamp_value(0s, 10s))
    DEFINE_TRANSFORMER(duration_coefficient, clamp_value(0.f, 4.f))

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
                        field = game_option_transformer<Is>(*value);
                    } else {
                        throw game_option_error("INVALID_OPTION_VALUE");
                    }
                }} ... });
        }(std::make_index_sequence<reflect::size<game_options>()>());
        
        auto it = set_option_map.find(key);
        if (it == set_option_map.end()) {
            throw game_option_error("INVALID_OPTION_NAME");
        }
        
        it->second(*this, value);
    }
    
    game_options game_options::deserialize_json(const json::json &value) {
        game_options result{};
        if (value.IsObject()) {
            reflect::for_each<game_options>([&](auto I) {
                auto member_name = reflect::member_name<I, game_options>();
                json::json key(rapidjson::StringRef(member_name.data(), member_name.size()));
                if (auto it = value.FindMember(key); it != value.MemberEnd()) {
                    try {
                        auto &field = reflect::get<I>(result);

                        using option_type = reflect::member_type<I, game_options>;
                        field = game_option_transformer<I>(json::deserialize<option_type>(it->value));
                    } catch (const std::exception &error) {
                        // ignore errors.
                        // game_options are stored in the clients' application storage and we don't want them kicked out if it's invalid.
                    }
                }
            });
        }
        return result;
    }

}