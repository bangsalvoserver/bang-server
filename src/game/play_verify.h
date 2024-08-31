#ifndef __PLAY_VERIFY_H__
#define __PLAY_VERIFY_H__

#include "game_update.h"
#include "play_dispatch.h"

namespace banggame {

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