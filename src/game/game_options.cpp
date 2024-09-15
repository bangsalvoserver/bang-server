#include "game_options.h"

#include "cards/card_data.h"

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

    template<size_t I> struct game_option_transformer;

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

    template<size_t I, typename T>
    T transform_field(T &&value) {
        if constexpr (is_complete<banggame::game_option_transformer<I>>) {
            return banggame::game_option_transformer<I>{}(std::forward<T>(value));
        }
        return value;
    }
    
    banggame::game_options deserialize_game_options(const json &value) {
        banggame::game_options result = banggame::default_game_options;
        if (value.is_object()) {
            reflect::for_each<banggame::game_options>([&](auto I) {
                auto member_name = reflect::member_name<I, banggame::game_options>();
                if (auto it = value.find(member_name); it != value.end()) {
                    try {
                        auto &field = reflect::get<I>(result);

                        using option_type = reflect::member_type<I, banggame::game_options>;
                        field = transform_field<I>(deserialize<option_type>(*it));
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