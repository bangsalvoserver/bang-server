#include "game_options.h"

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

}

namespace json {
    
    banggame::game_options deserialize_game_options(const json &value) {
        banggame::game_options result = banggame::default_game_options;
        if (value.is_object()) {
            reflect::for_each<banggame::game_options>([&](auto I) {
                auto member_name = reflect::member_name<I, banggame::game_options>();
                if (auto it = value.find(std::string(member_name)); it != value.end()) {
                    try {
                        reflect::get<I>(result) = deserialize<reflect::member_type<I, banggame::game_options>>(*it);
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