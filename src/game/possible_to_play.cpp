#include "possible_to_play.h"

#include "effects/base/equip.h"

namespace banggame {

    bool possible_to_play::check_recurse(card_ptr origin_card, effect_list_type type, size_t skip_targets) {
        const effect_list &effects = origin_card->get_effect_list(type);
        
        if (targets.size() == effects.size() + skip_targets) {
            if (type == effect_list_type::equip_effects) {
                if (!ctx.contains<contexts::equip_target>() && effect_equip_on{}.get_error(origin_card, origin, origin, ctx)) {
                    return false;
                }
            } else {
                if (const mth_holder &mth = origin_card->get_mth(type)) {
                    if (mth.get_error(origin_card, origin, targets_view{targets}.subspan(skip_targets), ctx)) {
                        return false;
                    }
                }

                if (const modifier_holder &modifier = origin_card->get_modifier(type)) {
                    bool found = false;
                    size_t old_size = ctx.size();

                    modifier.add_context(origin_card, origin, ctx);
                    modifiers.emplace_back(origin_card, type);

                    for (card_ptr target_card : get_all_active_cards(origin, ctx)) {
                        if (check(target_card, type)) {
                            found = true;
                            break;
                        }
                    }

                    modifiers.pop_back();
                    ctx.resize(old_size);
                    return found;
                }
            }

            if (verify_context(origin, origin_card, ctx)) {
                return false;
            }

            return true;
        }
        
        const effect_holder &effect = effects[targets.size() - skip_targets];

        for (play_card_target target : effect.possible_targets(origin_card, origin, ctx)) {
            size_t old_size = ctx.size();
            effect.add_context(origin_card, origin, target, ctx);
            targets.emplace_back(std::move(target));

            bool found = check_recurse(origin_card, type, skip_targets);
            
            targets.pop_back();
            ctx.resize(old_size);
            
            if (found) return true;
        }
        
        return false;
    }

    bool possible_to_play::check_impl(card_ptr origin_card, effect_list_type &type) {
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

        return check_recurse(origin_card, type, targets.size());
    }

    void possible_to_play::collect_recurse(card_ptr origin_card, effect_list_type type, playable_cards_list &result) {
        if (!check_impl(origin_card, type)) return;

        if (type == effect_list_type::equip_effects || !origin_card->get_modifier(type)) {
            if (modifiers.empty()) {
                result.emplace_back(origin_card, type);
            } else {
                result.emplace_back(origin_card, type, modifiers, ctx.get_serializable());
            }
        } else if (const modifier_holder &modifier = origin_card->get_modifier(type)) {
            size_t old_size = ctx.size();
            modifier.add_context(origin_card, origin, ctx);
            modifiers.emplace_back(origin_card, type);

            for (card_ptr target_card : get_all_active_cards(origin, ctx)) {
                collect_recurse(target_card, type, result);
            }

            modifiers.pop_back();
            ctx.resize(old_size);
        }
    }

    playable_cards_list generate_playable_cards_list(player_ptr origin, effect_list_type type) {
        playable_cards_list result;

        if (origin) {
            possible_to_play state{origin};

            for (card_ptr origin_card : get_all_active_cards(origin, state.ctx)) {
                state.collect_recurse(origin_card, type, result);
            }
        }

        return result;
    }
}