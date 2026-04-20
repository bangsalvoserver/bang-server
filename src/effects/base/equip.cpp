#include "equip.h"

#include "game/filters.h"
#include "game/prompts.h"
#include "game/bot_suggestion.h"

#include "cards/filter_enums.h"
#include "cards/game_events.h"

#include "target_types/base/player.h"

namespace banggame {

    const player_filter_bitset *get_equip_filter(const_card_ptr origin_card) {
        for (const effect_holder &holder : origin_card->equip_effects) {
            if (holder.type == GET_EFFECT(equip_on) && holder.target == TARGET_TYPE(player)) {
                return &static_cast<const targeting_player *>(holder.target_value)->player_filter;
            }
        }
        return nullptr;
    }

    game_string get_equip_error(const_card_ptr origin_card, player_ptr target) {
        if (const player_filter_bitset *filter = get_equip_filter(origin_card)) {
            MAYBE_RETURN(check_player_filter(origin_card, target, *filter, target));
        } else if (!target->alive()) {
            return {"ERROR_TARGET_DEAD", origin_card, target};
        }
        if (card_ptr equipped = target->find_equipped_card(origin_card)) {
            return {"ERROR_DUPLICATED_CARD", equipped};
        }
        return {};
    }
    
    game_string effect_equip_on::get_error(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) {
        if (card_ptr equipped = target->find_equipped_card(origin_card)) {
            return {"ERROR_DUPLICATED_CARD", equipped};
        }
        return origin->m_game->call_event(event_type::check_equip_card{ origin, origin_card, target, ctx });
    }

    void effect_equip_on::add_context(card_ptr origin_card, player_ptr origin, player_ptr target, effect_context &ctx) {
        ctx.set<contexts::equip_target>(target);
    }

    prompt_string effect_equip_on::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        return prompts::select_prompt(origin_card->equips | rv::transform([&](const equip_holder &holder) {
            return holder.on_prompt(origin_card, origin, target);
        }));
    }

    void effect_equip_on::on_play(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) {
        if (origin != target) {
            if (origin_card->has_tag(tag_type::penalty)) {
                bot_suggestion::signal_hostile_action(origin, target);
            } else {
                bot_suggestion::signal_helpful_action(origin, target);
            }
        }
        
        origin->m_game->queue_action([=]{ 
            if (!origin->alive()) return;

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
        }, 45);
    }

}