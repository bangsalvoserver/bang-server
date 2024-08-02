#ifndef __PLAY_VERIFY_H__
#define __PLAY_VERIFY_H__

#include "target_types/play_visitor.h"

#include "game_update.h"

namespace banggame {

    namespace play_dispatch {
        bool possible(player_ptr origin, card_ptr origin_card, const effect_holder &effect, const effect_context &ctx);
        play_card_target random_target(player_ptr origin, card_ptr origin_card, const effect_holder &effect, const effect_context &ctx);
        game_string get_error(player_ptr origin, card_ptr origin_card, const effect_holder &effect, const effect_context &ctx, const play_card_target &target);
        game_string prompt(player_ptr origin, card_ptr origin_card, const effect_holder &effect, const effect_context &ctx, const play_card_target &target);
        void add_context(player_ptr origin, card_ptr origin_card, const effect_holder &effect, effect_context &ctx, const play_card_target &target);
        void play(player_ptr origin, card_ptr origin_card, const effect_holder &effect, const effect_context &ctx, const play_card_target &target);
    }

    enum class message_type { ok, error, prompt };

    struct game_message {
        message_type type;
        game_string message;
    };

    game_string get_play_card_error(player_ptr origin, card_ptr origin_card, const effect_context &ctx);

    game_string get_equip_error(player_ptr origin, card_ptr origin_card, const_player_ptr target, const effect_context &ctx);

    game_string get_equip_prompt(player_ptr origin, card_ptr origin_card, player_ptr target);

    game_message verify_and_play(player_ptr origin, const game_action &action);

}

#endif