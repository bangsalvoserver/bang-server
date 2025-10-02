#include "possible_to_play.h"

namespace banggame {

    static bool is_possible_recurse(player_ptr origin, card_ptr origin_card, bool is_response, const card_list &modifiers, const effect_context &ctx, target_list &targets) {
        const effect_list &effects = origin_card->get_effect_list(is_response);
        
        if (targets.size() == effects.size()) {
            if (const mth_holder &mth = origin_card->get_mth(is_response)) {
                if (mth.get_error(origin_card, origin, targets, ctx)) {
                    return false;
                }
            }

            if (const modifier_holder &modifier = origin_card->get_modifier(is_response)) {
                auto modifiers_copy = modifiers;
                modifiers_copy.push_back(origin_card);

                auto ctx_copy = ctx;
                ctx_copy.selected_cards.push_back(origin_card);
                modifier.add_context(origin_card, origin, ctx_copy);
                
                return !rn::empty(get_all_playable_cards(origin, is_response, modifiers_copy, ctx_copy));
            }

            return !check_duplicates(ctx);
        }
        
        const effect_holder &effect = effects[targets.size()];

        for (play_card_target target : effect.possible_targets(origin_card, origin, ctx)) {
            auto ctx_copy = ctx;
            effect.add_context(origin_card, origin, target, ctx_copy);

            targets.emplace_back(std::move(target));
            if (is_possible_recurse(origin, origin_card, is_response, modifiers, ctx_copy, targets)) {
                return true;
            }
            targets.pop_back();
        }
        
        return false;
    }

    bool is_possible_to_play(player_ptr origin, card_ptr origin_card, bool is_response, const card_list &modifiers, const effect_context &ctx) {
        for (card_ptr mod_card : modifiers) {
            if (mod_card == origin_card) return false;
            if (mod_card->get_modifier(is_response).get_error(mod_card, origin, origin_card, ctx)) return false;
        }

        if (get_play_card_error(origin, origin_card, ctx)) {
            return false;
        }

        if (origin_card->is_equip_card()) {
            return !is_response && !rn::empty(get_all_equip_targets(origin, origin_card, ctx));
        } else {
            const effect_list &effects = origin_card->get_effect_list(is_response);
            if (effects.empty()) return false;
            
            for (const effect_holder &effect : effects) {
                if (!effect.can_play(origin_card, origin, ctx)) {
                    return false;
                }
            }

            target_list targets;
            targets.reserve(effects.size());
            return is_possible_recurse(origin, origin_card, is_response, modifiers, ctx, targets);
        }
    }

    static void collect_playable_cards(
        playable_cards_list &result, card_list &modifiers,
        player_ptr origin, card_ptr origin_card, bool is_response, effect_context ctx
    ) {
        const modifier_holder &modifier = origin_card->get_modifier(is_response);
        if (origin_card->is_equip_card() || !modifier) {
            if (modifiers.empty()) {
                result.emplace_back(origin_card);
            } else {
                result.emplace_back(origin_card, modifiers, ctx);
            }
        } else {
            modifier.add_context(origin_card, origin, ctx);

            modifiers.push_back(origin_card);
            for (card_ptr target_card : get_all_playable_cards(origin, is_response, modifiers, ctx)) {
                collect_playable_cards(result, modifiers, origin, target_card, is_response, ctx);
            }
            modifiers.pop_back();
        }
    }

    playable_cards_list generate_playable_cards_list(player_ptr origin, bool is_response) {
        playable_cards_list result;
        card_list modifiers;

        if (origin) {
            for (card_ptr origin_card : get_all_playable_cards(origin, is_response)) {
                collect_playable_cards(result, modifiers, origin, origin_card, is_response, {});
            }
        }

        return result;
    }
}