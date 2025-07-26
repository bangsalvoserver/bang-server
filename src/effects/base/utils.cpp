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
    
    void effect_set_playing::on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        switch (type) {
        case play_as::missed:
            origin->m_game->add_log("LOG_PLAYED_CARD_AS_MISSED", target_card, origin);
            break;
        case play_as::gatling:
            origin->m_game->add_log("LOG_PLAYED_CARD_AS_GATLING", target_card, origin);
            break;
        }
        origin->discard_used_card(target_card);
    }

    void equip_add_flag::on_enable(card_ptr origin_card, player_ptr target) {
        target->add_player_flags(flag);
    }

    void equip_add_flag::on_disable(card_ptr origin_card, player_ptr target) {
        target->remove_player_flags(flag);
    }

    void equip_game_flag::on_enable(card_ptr origin_card, player_ptr target) {
        target->m_game->add_game_flags(flag);
    }

    void equip_game_flag::on_disable(card_ptr origin_card, player_ptr target) {
        target->m_game->remove_game_flags(flag);
    }

}