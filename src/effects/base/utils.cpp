#include "utils.h"

#include "game/game_table.h"
#include "game/play_verify.h"

namespace banggame {
    
    void event_equip::on_disable(card_ptr target_card, player_ptr target) {
        target->m_game->remove_listeners(target_card);
    }

    bool effect_human::can_play(card_ptr origin_card, player_ptr origin) {
        return !origin->is_bot();
    }

    void effect_set_playing::add_context(card_ptr origin_card, player_ptr origin, card_ptr target, effect_context &ctx) {
        ctx.playing_card = target;
    }
    
    game_string effect_set_playing::get_error(card_ptr origin_card, player_ptr origin, card_ptr target, const effect_context &ctx) {
        return get_play_card_error(origin, target, ctx);
    }

}