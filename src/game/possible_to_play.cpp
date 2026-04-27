#include "possible_to_play.h"

namespace banggame {

    static bool is_possible_recurse(player_ptr origin, card_ptr origin_card, bool is_response, card_response_list &modifiers, const effect_context &ctx, target_list &targets) {
        bool is_equip = origin_card->is_equip_card();
        const effect_list &effects = is_equip ? origin_card->equip_effects : origin_card->get_effect_list(is_response);
        
        if (targets.size() == effects.size()) {
            if (!is_equip) {
                if (const mth_holder &mth = origin_card->get_mth(is_response)) {
                    if (mth.get_error(origin_card, origin, targets, ctx)) {
                        return false;
                    }
                }

                if (const modifier_holder &modifier = origin_card->get_modifier(is_response)) {
                    auto ctx_copy = ctx;
                    ctx_copy.add<contexts::selected_cards>().push_back(origin_card);
                    modifier.add_context(origin_card, origin, ctx_copy);

                    modifiers.emplace_back(origin_card, is_response);
                    for (card_ptr target_card : get_all_active_cards(origin, ctx_copy)) {
                        bool is_response_copy = is_response && ctx_copy.get<contexts::forced_play>() != target_card;
                        if (is_possible_to_play(origin, target_card, is_response_copy, modifiers, ctx_copy)) {
                            modifiers.pop_back();
                            return true;
                        }
                    }
                    modifiers.pop_back();
                    return false;
                }
            }

            return !verify_context(origin, origin_card, ctx);
        }
        
        const effect_holder &effect = effects[targets.size()];

        for (play_card_target target : effect.possible_targets(origin_card, origin, ctx)) {
            auto ctx_copy = ctx;
            effect.add_context(origin_card, origin, target, ctx_copy);

            targets.emplace_back(std::move(target));
            if (is_possible_recurse(origin, origin_card, is_response, modifiers, ctx_copy, targets)) {
                targets.pop_back();
                return true;
            }
            targets.pop_back();
        }
        
        return false;
    }

    bool is_possible_to_play(player_ptr origin, card_ptr origin_card, bool is_response, card_response_list &modifiers, const effect_context &ctx) {
        for (auto [mod_card, is_mod_response] : modifiers) {
            if (mod_card == origin_card) return false;
            if (mod_card->get_modifier(is_mod_response).get_error(mod_card, origin, origin_card, ctx)) return false;
        }

        if (get_play_card_error(origin, origin_card, ctx)) {
            return false;
        }

        bool is_equip = origin_card->is_equip_card();
        const effect_list &effects = is_equip ? origin_card->equip_effects : origin_card->get_effect_list(is_response);

        if (is_equip ? is_response : effects.empty()) return false;

        for (const effect_holder &effect : effects) {
            if (!effect.can_play(origin_card, origin, ctx)) {
                return false;
            }
        }

        target_list targets;
        targets.reserve(effects.size());
        return is_possible_recurse(origin, origin_card, is_response, modifiers, ctx, targets);
    }

    static void collect_playable_cards(
        playable_cards_list &result, card_response_list &modifiers,
        player_ptr origin, card_ptr origin_card, bool is_response, const effect_context &ctx
    ) {
        const modifier_holder &modifier = origin_card->get_modifier(is_response);
        if (origin_card->is_equip_card() || !modifier) {
            if (modifiers.empty()) {
                result.emplace_back(origin_card, is_response);
            } else {
                result.emplace_back(origin_card, is_response, modifiers, ctx);
            }
        } else {
            effect_context ctx_copy = ctx;
            modifier.add_context(origin_card, origin, ctx_copy);

            modifiers.emplace_back(origin_card, is_response);
            for (card_ptr target_card : get_all_active_cards(origin, ctx_copy)) {
                bool is_response_copy = is_response && ctx_copy.get<contexts::forced_play>() != target_card;
                if (is_possible_to_play(origin, target_card, is_response_copy, modifiers, ctx_copy)) {
                    collect_playable_cards(result, modifiers, origin, target_card, is_response_copy, ctx_copy);
                }
            }
            modifiers.pop_back();
        }
    }

    playable_cards_list generate_playable_cards_list(player_ptr origin, bool is_response) {
        playable_cards_list result;
        
        if (origin) {
            effect_context ctx{};
            card_response_list modifiers;

            for (card_ptr origin_card : get_all_active_cards(origin, ctx)) {
                if (is_possible_to_play(origin, origin_card, is_response, modifiers, ctx)) {
                    collect_playable_cards(result, modifiers, origin, origin_card, is_response, ctx);
                }
            }
        }

        return result;
    }
}