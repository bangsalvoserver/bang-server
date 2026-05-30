#include "possible_to_play.h"

#include "effects/base/equip.h"

namespace banggame {

    static bool is_possible_impl(player_ptr origin, card_ptr origin_card, effect_list_type &type, playable_card_list &modifiers, const effect_context &ctx);

    bool is_possible_to_play(player_ptr origin, card_ptr origin_card, effect_list_type type, const effect_context &ctx) {
        playable_card_list modifiers;
        return is_possible_impl(origin, origin_card, type, modifiers, ctx);
    }

    static bool is_possible_recurse(player_ptr origin, card_ptr origin_card, effect_list_type type, playable_card_list &modifiers, const effect_context &ctx, target_list &targets) {
        const effect_list &effects = origin_card->get_effect_list(type);
        
        if (targets.size() == effects.size()) {
            if (type == effect_list_type::equip_effects) {
                if (!ctx.contains<contexts::equip_target>() && effect_equip_on{}.get_error(origin_card, origin, origin, ctx)) {
                    return false;
                }
            } else {
                if (const mth_holder &mth = origin_card->get_mth(type)) {
                    if (mth.get_error(origin_card, origin, targets, ctx)) {
                        return false;
                    }
                }

                if (const modifier_holder &modifier = origin_card->get_modifier(type)) {
                    auto ctx_copy = ctx;
                    modifier.add_context(origin_card, origin, ctx_copy);

                    modifiers.emplace_back(origin_card, type);
                    for (card_ptr target_card : get_all_active_cards(origin, ctx_copy)) {
                        auto type_copy = type;
                        if (is_possible_impl(origin, target_card, type_copy, modifiers, ctx_copy)) {
                            modifiers.pop_back();
                            return true;
                        }
                    }
                    modifiers.pop_back();
                    return false;
                }
            }

            if (verify_context(origin, origin_card, modifiers | rv::transform(&playable_card_entry::card) | rn::to<card_list>(), ctx)) {
                return false;
            }

            return true;
        }
        
        const effect_holder &effect = effects[targets.size()];

        for (play_card_target target : effect.possible_targets(origin_card, origin, ctx)) {
            auto ctx_copy = ctx;
            effect.add_context(origin_card, origin, target, ctx_copy);

            targets.emplace_back(std::move(target));
            if (is_possible_recurse(origin, origin_card, type, modifiers, ctx_copy, targets)) {
                targets.pop_back();
                return true;
            }
            targets.pop_back();
        }
        
        return false;
    }

    static bool is_possible_impl(player_ptr origin, card_ptr origin_card, effect_list_type &type, playable_card_list &modifiers, const effect_context &ctx) {
        if (type == effect_list_type::responses && ctx.get<contexts::forced_play>() == origin_card) {
            type = effect_list_type::effects;
        }

        if (origin_card->is_equip_card()) {
            if (type == effect_list_type::effects) {
                type = effect_list_type::equip_effects;
            } else {
                return false;
            }
        }

        for (auto [mod_card, mod_type] : modifiers) {
            if (mod_card == origin_card) return false;
            if (mod_card->get_modifier(mod_type).get_error(mod_card, origin, origin_card, ctx)) return false;
        }

        if (get_play_card_error(origin, origin_card, ctx)) {
            return false;
        }

        const effect_list &effects = origin_card->get_effect_list(type);

        if (type != effect_list_type::equip_effects && effects.empty()) return false;

        for (const effect_holder &effect : effects) {
            if (!effect.can_play(origin_card, origin, ctx)) {
                return false;
            }
        }

        target_list targets;
        targets.reserve(effects.size());
        return is_possible_recurse(origin, origin_card, type, modifiers, ctx, targets);
    }

    static void collect_playable_cards(
        playable_cards_list &result, playable_card_list &modifiers,
        player_ptr origin, card_ptr origin_card, effect_list_type type, const effect_context &ctx
    ) {
        if (type == effect_list_type::equip_effects || !origin_card->get_modifier(type)) {
            if (modifiers.empty()) {
                result.emplace_back(origin_card, type);
            } else {
                result.emplace_back(origin_card, type, modifiers, ctx);
            }
        } else if (const modifier_holder &modifier = origin_card->get_modifier(type)) {
            effect_context ctx_copy = ctx;
            modifier.add_context(origin_card, origin, ctx_copy);

            modifiers.emplace_back(origin_card, type);
            for (card_ptr target_card : get_all_active_cards(origin, ctx_copy)) {
                auto type_copy = type;
                if (is_possible_impl(origin, target_card, type_copy, modifiers, ctx_copy)) {
                    collect_playable_cards(result, modifiers, origin, target_card, type_copy, ctx_copy);
                }
            }
            modifiers.pop_back();
        }
    }

    playable_cards_list generate_playable_cards_list(player_ptr origin, effect_list_type type) {
        playable_cards_list result;
        
        if (origin) {
            effect_context ctx{};
            playable_card_list modifiers;

            for (card_ptr origin_card : get_all_active_cards(origin, ctx)) {
                auto type_copy = type;
                if (is_possible_impl(origin, origin_card, type_copy, modifiers, ctx)) {
                    collect_playable_cards(result, modifiers, origin, origin_card, type_copy, ctx);
                }
            }
        }

        return result;
    }
}