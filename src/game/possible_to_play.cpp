#include "possible_to_play.h"

namespace banggame {

    static bool is_possible_recurse(player_ptr origin, card_ptr origin_card, const effect_list &effects, const mth_holder &mth, effect_context &ctx, target_list &targets) {
        if (targets.size() == effects.size()) {
            return !(mth && mth.get_error(origin_card, origin, targets, ctx))
                && !check_duplicates(ctx);
        }

        const effect_holder &effect = effects[targets.size()];
        if (!effect.can_play(origin_card, origin, ctx)) {
            return false;
        }

        return play_dispatch::any_of_possible_targets(origin, origin_card, effect, ctx, [&](const play_card_target &target) {
            auto ctx_copy = ctx;
            play_dispatch::add_context(origin, origin_card, effect, ctx_copy, target);

            targets.emplace_back(target);
            bool result = is_possible_recurse(origin, origin_card, effects, mth, ctx_copy, targets);
            targets.pop_back();

            if (result) ctx = ctx_copy;
            return result;
        });
    }

    static auto map_cards_playable_with_modifiers(
        player_ptr origin, const card_list &modifiers, bool is_response, const effect_context &ctx,
        auto function
    ) {
        auto map = [&](rn::forward_range auto &&range) {
            return function(rv::filter(std::forward<decltype(range)>(range), [&](card_ptr origin_card) {
                return is_possible_to_play(origin, origin_card, is_response, modifiers, ctx);
            }));
        };

        if (ctx.card_choice) {
            return map(origin->m_game->m_hidden_deck);
        } else if (ctx.traincost) {
            return map(origin->m_game->m_train);
        } else if (ctx.repeat_card) {
            return map(rv::single(ctx.repeat_card));
        } else {
            return map(get_all_active_cards(origin));
        }
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
            if (is_response || get_all_equip_targets(origin, origin_card, ctx).empty()) {
                return false;
            }
        } else {
            const effect_list &effects = origin_card->get_effect_list(is_response);
            if (effects.empty()) return false;

            const mth_holder &mth = origin_card->get_mth(is_response);

            auto ctx_copy = ctx;

            target_list targets;
            targets.reserve(effects.size());
            if (!is_possible_recurse(origin, origin_card, effects, mth, ctx_copy, targets)) {
                return false;
            }

            if (const modifier_holder &modifier = origin_card->get_modifier(is_response)) {
                auto modifiers_copy = modifiers;
                modifiers_copy.push_back(origin_card);
                modifier.add_context(origin_card, origin, ctx_copy);
                
                return map_cards_playable_with_modifiers(origin, modifiers_copy, is_response, ctx_copy, std::not_fn(rn::empty));
            }
        }

        return true;
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
            map_cards_playable_with_modifiers(origin, modifiers, is_response, ctx, [&](auto &&range) {
                for (card_ptr target_card : range) {
                    collect_playable_cards(result, modifiers, origin, target_card, is_response, ctx);
                }
            });
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