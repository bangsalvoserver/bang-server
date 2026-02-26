#ifndef __GAME_OPTIONS_H__
#define __GAME_OPTIONS_H__

#include "durations.h"
#include "expansion_set.h"
#include "cards/card_fwd.h"

namespace banggame {
    
    using namespace std::chrono_literals;

    struct game_option_error : std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    template<typename T>
    struct transform {
        T transformer;
    };

    inline expansion_set transform_expansions(const expansion_set &expansions) {
        if (!validate_expansions(expansions)) {
            throw game_option_error("INVALID_EXPANSIONS");
        }
        return expansions;
    }

    template<typename T>
    constexpr auto clamp_value(T min, T max) {
        return [=](T value) {
            return std::clamp(value, min, max);
        };
    }

    constexpr auto clamp_seconds(int min, int max) {
        return [=]<typename Rep, typename Period>(std::chrono::duration<Rep, Period> value) {
            using duration_type = decltype(value);
            auto min_duration = std::chrono::duration_cast<duration_type>(std::chrono::seconds(min));
            auto max_duration = std::chrono::duration_cast<duration_type>(std::chrono::seconds(max));
            return std::clamp(value, min_duration, max_duration);
        };
    }
    
    struct game_options {
        [[=transform(transform_expansions)]]
        expansion_set expansions;

        [[=transform(clamp_value(1, 3))]]
        int character_choice = 1;

        [[=transform(clamp_value(3, 8))]]
        int max_players = 5;
        
        bool add_bots = false;
        bool quick_discard_all = true;
        bool auto_pick_predraw = true;
        bool allow_bot_rejoin = false;
        bool only_base_characters = false;

        [[=transform(clamp_value(0, 30))]]
        int scenario_deck_size = 12;

        [[=transform(clamp_seconds(0, 5))]]
        game_duration auto_resolve_timer = 1000ms;

        [[=transform(clamp_seconds(0, 5))]]
        game_duration damage_timer = 1500ms;

        [[=transform(clamp_seconds(0, 10))]]
        game_duration escape_timer = 3000ms;

        [[=transform(clamp_seconds(0, 10))]]
        game_duration bot_play_timer = 500ms;

        [[=transform(clamp_value(0.f, 4.f))]]
        float duration_coefficient = 1.f;
        
        unsigned int game_seed = 0;
        unsigned int bot_rng_seed = 0;

        static game_options deserialize_json(const json::json &value);

        std::string to_string(std::string_view sep = "\n") const;
        void set_option(std::string_view key, std::string_view value);
    };

}

namespace json {

    template<typename Context>
    struct deserializer<banggame::game_options, Context> {
        static banggame::game_options read(const json &value, const Context &ctx) {
            return banggame::game_options::deserialize_json(value);
        }
    };
}

#endif