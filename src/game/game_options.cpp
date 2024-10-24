#include "game_options.h"

#include "cards/expansion_set.h"
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

    template<size_t I> struct game_option_transformer {
        template<typename T>
        T operator()(const T &value) const {
            return value;
        }
    };

    template<> struct game_option_transformer<game_option_field_index("expansions")> {
        expansion_set operator()(const expansion_set &value) const {
            if (!validate_expansions(value)) {
                throw std::runtime_error("INVALID_EXPANSIONS");
            }
            return value;
        }
    };

    template<> struct game_option_transformer<game_option_field_index("damage_timer")> {
        game_duration operator()(game_duration value) const {
            return std::clamp(value, game_duration{}, game_duration{5s});
        }
    };

    template<> struct game_option_transformer<game_option_field_index("escape_timer")> {
        game_duration operator()(game_duration value) const {
            return std::clamp(value, game_duration{}, game_duration{10s});
        }
    };

    template<> struct game_option_transformer<game_option_field_index("bot_play_timer")> {
        game_duration operator()(game_duration value) const {
            return std::clamp(value, game_duration{}, game_duration{10s});
        }
    };

    template<> struct game_option_transformer<game_option_field_index("tumbleweed_timer")> {
        game_duration operator()(game_duration value) const {
            return std::clamp(value, game_duration{}, game_duration{10s});
        }
    };

    template<> struct game_option_transformer<game_option_field_index("duration_coefficient")> {
        float operator()(float value) const {
            return std::clamp(value, 0.f, 4.f);
        }
    };

    template<size_t I, typename T>
    T transform_field(const T &value) {
        return banggame::game_option_transformer<I>{}(value);
    }

    std::string game_options::to_string() const {
        std::string result;
        reflect::for_each<game_options>([&](auto I) {
            if constexpr (I != 0) {
                result += '\n';
            }
            result += std::format("{} = {}", reflect::member_name<I>(*this), reflect::get<I>(*this));
        });
        return result;
    }

    void game_options::set_option(std::string_view key, std::string_view value) {
        static constexpr auto set_option_map = []<size_t ... Is>(std::index_sequence<Is ...>){
            using set_option_fn_ptr = void (*)(game_options &options, std::string_view value_str);

            return utils::static_map<std::string_view, set_option_fn_ptr>({
                { reflect::member_name<Is, game_options>(), [](game_options &options, std::string_view value_str) {
                    auto &field = reflect::get<Is>(options);
                    if (auto value = utils::parse_string<std::remove_reference_t<decltype(field)>>(value_str)) {
                        field = transform_field<Is>(*value);
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
                        field = transform_field<I>(json::deserialize<option_type>(*it));
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