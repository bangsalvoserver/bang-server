#include "equip.h"

#include "game/game.h"
#include "game/play_verify.h"

namespace banggame {

    game_string handler_equip::get_error(card *origin_card, player *origin, const effect_context &ctx, card *target_card, player *target) {
        return get_play_card_error(origin, target_card, ctx);
    }
    
    game_string handler_equip::on_prompt(card *origin_card, player *origin, const effect_context &ctx, card *target_card, player *target) {
        for (const equip_holder &holder : target_card->equips) {
            MAYBE_RETURN(holder.type->on_prompt(holder.effect_value, target_card, origin, target));
        }
        return {};
    }

    static void apply_equip(card *origin_card, player *origin, const effect_context &ctx, player *target) {
        if (origin_card->pocket == pocket_type::shop_selection) {
            if (origin == target) {
                origin->m_game->add_log("LOG_BOUGHT_EQUIP", origin_card, origin);
            } else {
                origin->m_game->add_log("LOG_BOUGHT_EQUIP_TO", origin_card, origin, target);
            }
        } else {
            if (origin == target) {
                origin->m_game->add_log("LOG_EQUIPPED_CARD", origin_card, origin);
            } else {
                origin->m_game->add_log("LOG_EQUIPPED_CARD_TO", origin_card, origin, target);
            }
        }
        
        if (origin_card->pocket == pocket_type::player_hand) {
            origin->m_game->call_event(event_type::on_discard_hand_card{ origin, origin_card, true });
        }

        target->equip_card(origin_card);

        origin->m_game->call_event(event_type::on_equip_card{ origin, target, origin_card, ctx });
    }

    void handler_equip::on_play(card *origin_card, player *origin, const effect_context &ctx, card *target_card, player *target) {
        origin->m_game->queue_action([=]{
            if (origin->alive()) {
                apply_equip(target_card, origin, ctx, target);
            }
        });
    }
}