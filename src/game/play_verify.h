#ifndef __PLAY_VERIFY_H__
#define __PLAY_VERIFY_H__

#include "cards/card_defs.h"

namespace banggame {

    game_string verify_context(player_ptr origin, card_ptr origin_card, const effect_context &ctx);

    game_string get_play_card_error(player_ptr origin, card_ptr origin_card, const effect_context &ctx);

    namespace play_verify_results {
        struct ok {};

        struct error {
            game_string message;
        };

        struct prompt {
            prompt_string message;
        };
    }

    using play_verify_result = std::variant<
        play_verify_results::ok,
        play_verify_results::error,
        play_verify_results::prompt
    >;

    struct target_selection {
        card_ptr card;
        bool is_response;
        target_list targets;
    };

    using modifier_list = std::vector<target_selection>;

    struct game_action {
        card_ptr card;
        bool is_response;
        target_list targets;

        modifier_list modifiers;

        bool bypass_prompt;
    };

    play_verify_result verify_and_play(player_ptr origin, const game_action &action);

}

#endif