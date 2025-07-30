#ifndef __PLAY_VERIFY_H__
#define __PLAY_VERIFY_H__

#include "game_update.h"

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

    play_verify_result verify_and_play(player_ptr origin, const game_action &action);

    template<rn::input_range R> requires std::convertible_to<rn::range_value_t<R>, prompt_string>
    inline prompt_string merge_prompts(R &&prompts, bool ignore_empty = false) {
        prompt_string result;
        bool empty = false;
        for (const prompt_string &str : prompts) {
            if (!str) {
                empty = true;
            } else if (!result || str.priority > result.priority) {
                result = str;
            }
        }
        if (empty && result.priority == 0 && !ignore_empty) {
            return {};
        }
        return result;
    }

}

#endif