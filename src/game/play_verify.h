#ifndef __PLAY_VERIFY_H__
#define __PLAY_VERIFY_H__

#include "cards/card_defs.h"

namespace banggame {

    game_string check_duplicates(const effect_context &ctx);

    game_string get_play_card_error(player_ptr origin, card_ptr origin_card, const effect_context &ctx);

    game_string get_equip_error(player_ptr origin, card_ptr origin_card, const_player_ptr target, const effect_context &ctx);

    prompt_string get_equip_prompt(player_ptr origin, card_ptr origin_card, player_ptr target);

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

    struct card_targets_pair {
        card_ptr card;
        target_list targets;
    };

    using modifier_list = std::vector<card_targets_pair>;

    struct game_action {
        card_ptr card;
        modifier_list modifiers;
        target_list targets;
        bool bypass_prompt;
    };

    play_verify_result verify_and_play(player_ptr origin, const game_action &action);

}

#endif