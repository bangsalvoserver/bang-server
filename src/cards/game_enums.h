#ifndef __CARDS_GAME_ENUMS_H__
#define __CARDS_GAME_ENUMS_H__

#include "utils/enum_bitset.h"
#include "utils/misc.h"

namespace banggame {

    enum class sound_id {
        gamestart,
        victory,
        draw,
        death,
        bang,
        gatling,
        indians,
        duel,
        dynamite,
        generalstore,
        shuffle,
        bandidos,
        snake,
        train,
    };
    
    enum class effect_flag {
        is_bang,
        is_missed,
        play_as_bang,
        single_target,
        multi_target,
        target_players,
    };

    inline constexpr struct ignore_flag_t {} ignore_flag;

    template<enums::enumeral T>
    consteval enums::bitset<T> ignored_bitset() {
        enums::bitset<T> result;
        for (std::meta::info value : enums::enumerators<T>) {
            if (has_annotation(value, ^^ignore_flag_t)) {
                result.add(std::meta::extract<T>(value));
            }
        }
        return result;
    }
    
    enum class game_flag {
        game_over,
        invert_rotation [[=ignore_flag]],
        phase_one_draw_discard [[=ignore_flag]],
        phase_one_override [[=ignore_flag]],
        disable_player_distances,
        showdown,
        hands_shown,
        free_for_all [[=ignore_flag]],
    };

    static constexpr auto ignored_game_flags = ignored_bitset<game_flag>();

    enum class player_flag {
        dead,
        ghost_1,
        ghost_2,
        temp_ghost,
        shadow,
        extra_turn [[=ignore_flag]],
        stick_of_dynamite,
        treat_missed_as_bang,
        treat_any_as_bang,
        treat_any_as_missed,
        ignore_distances,
        role_revealed [[=ignore_flag]],
        show_hand_playing,
        skip_turn,
        legend [[=ignore_flag]],
        removed,
        winner,
        positive_karma [[=ignore_flag]],
        negative_karma [[=ignore_flag]],
    };

    static constexpr auto ignored_player_flags = ignored_bitset<player_flag>();
}

#endif