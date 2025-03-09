#ifndef __PLAY_VERIFY_H__
#define __PLAY_VERIFY_H__

#include "game_update.h"
#include "play_dispatch.h"

namespace banggame {

    game_string check_duplicates(const effect_context &ctx);

    game_string get_play_card_error(player_ptr origin, card_ptr origin_card, const effect_context &ctx);

    game_string get_equip_error(player_ptr origin, card_ptr origin_card, const_player_ptr target, const effect_context &ctx);

    prompt_string get_equip_prompt(player_ptr origin, card_ptr origin_card, player_ptr target);

    using game_message = utils::tagged_variant<
        utils::tag<"ok">,
        utils::tag<"error", game_string>,
        utils::tag<"prompt", prompt_string>
    >;

    game_message verify_and_play(player_ptr origin, const game_action &action);

    template<rn::input_range R> requires std::convertible_to<rn::range_value_t<R>, prompt_string>
    inline prompt_string merge_prompts(R &&prompts) {
        prompt_string result;
        bool empty = false;
        for (const prompt_string &str : prompts) {
            if (str.type == prompt_type::priority) {
                return str;
            } else if (!str) {
                empty = true;
            } else if (!result) {
                result = str;
            }
        }
        if (empty) {
            return {};
        }
        return result;
    }

}

#endif