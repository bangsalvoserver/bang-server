#ifndef __DURATIONS_H__
#define __DURATIONS_H__

#include <chrono>

namespace banggame {

    using game_duration = std::chrono::milliseconds;

    class animation_duration {
    private:
        game_duration value;
        
    public:
        animation_duration() = default;
        animation_duration(game_duration value) : value{value} {}

        game_duration get() const { return value; }
    };

    extern const struct durations_t {
        animation_duration move_token;
        animation_duration move_tokens;
        animation_duration move_card;
        animation_duration move_deck;
        animation_duration deck_shuffle;
        animation_duration flip_card;
        animation_duration tap_card;
        animation_duration flash_card;
        animation_duration short_pause;
        animation_duration move_player;
        animation_duration player_hp;
        animation_duration move_train;
    } durations;

}

#endif