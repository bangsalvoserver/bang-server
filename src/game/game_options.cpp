#include "game_options.h"

#include "cards/card_data.h"

#include "net/manager.h"

#include "utils/parse_string.h"
#include "utils/static_map.h"

template<> struct std::formatter<banggame::expansion_set> : std::formatter<std::string_view> {
    static std::string expansions_to_string(banggame::expansion_set value) {
        std::string ret;
        for (const banggame::ruleset_vtable *ruleset : banggame::all_cards.expansions) {
            if (value.contains(ruleset)) {
                if (!ret.empty()) {
                    ret += ' ';
                }
                ret.append(ruleset->name);
            }
        }
        return ret;
    }

    auto format(banggame::expansion_set value, std::format_context &ctx) const {
        return std::formatter<std::string_view>::format(expansions_to_string(value), ctx);
    }
};

template<> struct string_parser<banggame::expansion_set> {
    std::optional<banggame::expansion_set> operator()(std::string_view str) {
        constexpr std::string_view whitespace = " \t";
        banggame::expansion_set result;
        while (true) {
            size_t pos = str.find_first_not_of(whitespace);
            if (pos == std::string_view::npos) break;
            str = str.substr(pos);
            pos = str.find_first_of(whitespace);
            auto it = rn::find(banggame::all_cards.expansions, str.substr(0, pos), &banggame::ruleset_vtable::name);
            if (it != banggame::all_cards.expansions.end()) {
                result.insert(*it);
            } else {
                return std::nullopt;
            }
            if (pos == std::string_view::npos) break;
            str = str.substr(pos);
        }
        return result;
    }
};

namespace banggame {
    
    using namespace std::chrono_literals;

    const game_options default_game_options {
        .expansions { },
        .enable_ghost_cards { false },
        .character_choice { true },
        .quick_discard_all { true },
        .scenario_deck_size { 12 },
        .num_bots { 0 },
        .damage_timer { 1500ms },
        .escape_timer { 3000ms },
        .bot_play_timer { 500ms },
        .tumbleweed_timer { },
        .duration_coefficient { 1.f },
        .game_seed { 0 }
    };

    constexpr size_t game_option_field_index(std::string_view name) {
        size_t result = -1;
        reflect::for_each<game_options>([&](auto I) {
            if (reflect::member_name<I, game_options>() == name) {
                result = I;
            }
        });
        if (result != -1) {
            return result;
        }
        throw std::out_of_range("Cannot find game_option_field_index");
    }

    template<size_t I> struct game_option_transformer {
        template<typename T>
        T operator()(const T &value) const {
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

    void game_manager::command_get_game_options(game_user &user) {
        const game_options &options = user.in_lobby->options;
        reflect::for_each<game_options>([&](auto I) {
            send_message<"lobby_message">(user.client,
                std::format("{} = {}", reflect::member_name<I>(options), reflect::get<I>(options))
            );
        });
    }

    void game_manager::command_set_game_option(game_user &user, std::string_view name, std::string_view value) {
        static constexpr auto set_option_map = []<size_t ... Is>(std::index_sequence<Is ...>){
            using set_option_fn_ptr = bool (*)(game_options &options, std::string_view value_str);

            return utils::static_map<std::string_view, set_option_fn_ptr>({
                { reflect::member_name<Is, game_options>(), [](game_options &options, std::string_view value_str) {
                    auto &field = reflect::get<Is>(options);
                    if (auto value = parse_string<std::remove_reference_t<decltype(field)>>(value_str)) {
                        field = transform_field<Is>(*value);
                        return true;
                    } else {
                        return false;
                    }
                }} ... });
        }(std::make_index_sequence<reflect::size<game_options>()>());
        
        if (auto it = set_option_map.find(name); it != set_option_map.end()) {
            auto &lobby = *user.in_lobby;

            if (it->second(lobby.options, value)) {
                broadcast_message_lobby<"lobby_edited">(lobby, lobby);
            } else {
                throw lobby_error("INVALID_OPTION_VALUE");
            }
        } else {
            throw lobby_error("INVALID_OPTION_NAME");
        }
    }

    void game_manager::command_reset_game_options(game_user &user) {
        auto &lobby = *user.in_lobby;
        lobby.options = banggame::default_game_options;
        broadcast_message_lobby<"lobby_edited">(lobby, lobby);
    }

}

namespace json {

    template<typename Context> struct deserializer<banggame::expansion_set, Context> {
        banggame::expansion_set operator()(const json &value) const {
            if (!value.is_array()) {
                throw deserialize_error("Cannot deserialize expansion_set");
            }

            banggame::expansion_set result;
            for (const banggame::ruleset_vtable *ruleset : banggame::all_cards.expansions) {
                if (rn::contains(value, ruleset->name, [](const json &name) { return name.get<std::string_view>(); })) {
                    result.insert(ruleset);
                }
            }
            return result;
        }
    };
    
    banggame::game_options deserialize_game_options(const json &value) {
        banggame::game_options result = banggame::default_game_options;
        if (value.is_object()) {
            reflect::for_each<banggame::game_options>([&](auto I) {
                auto member_name = reflect::member_name<I, banggame::game_options>();
                if (auto it = value.find(member_name); it != value.end()) {
                    try {
                        auto &field = reflect::get<I>(result);

                        using option_type = reflect::member_type<I, banggame::game_options>;
                        field = banggame::transform_field<I>(deserialize<option_type>(*it));
                    } catch (const deserialize_error &error) {
                        // ignore errors.
                        // game_options are stored in the clients' application storage and we don't want them kicked out if it's invalid.
                    }
                }
            });
        }
        return result;
    }
}