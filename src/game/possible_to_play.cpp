#include "possible_to_play.h"

namespace banggame {

    static bool is_possible_mth(player *origin, card *origin_card, const mth_holder &mth, const effect_list &effects, const effect_context &ctx, target_list &targets) {
        if (targets.size() == mth.args.size()) {
            return !mth_holder{
                mth.type,
                small_int_set(small_int_set_sized_tag, targets.size())
            }.get_error(origin_card, origin, targets, ctx);
        }
        const auto &effect = effects.at(mth.args[targets.size()]);
        if (effect.target == TARGET_TYPE(player)) {
            for (player *target : get_all_player_targets(origin, origin_card, effect, ctx)) {
                targets.emplace_back(utils::tag<"player">{}, target);
                bool result = is_possible_mth(origin, origin_card, mth, effects, ctx, targets);
                targets.pop_back();
                if (result) return true;
            }
            return false;
        } else if (effect.target == TARGET_TYPE(card)) {
            for (card *target : get_all_card_targets(origin, origin_card, effect, ctx)) {
                targets.emplace_back(utils::tag<"card">{}, target);
                bool result = is_possible_mth(origin, origin_card, mth, effects, ctx, targets);
                targets.pop_back();
                if (result) return true;
            }
            return false;
        } else {
            // ignore other target types
            return true;
        }
    }

    static auto map_cards_playable_with_modifiers(
        player *origin, const card_list &modifiers, bool is_response, const effect_context &ctx,
        auto function
    ) {
        auto map = [&](rn::forward_range auto &&range) {
            return function(rv::filter(std::forward<decltype(range)>(range), [&](card *origin_card) {
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

    bool is_possible_to_play(player *origin, card *origin_card, bool is_response, const card_list &modifiers, const effect_context &ctx) {
        for (card *mod_card : modifiers) {
            if (mod_card == origin_card) return false;
            if (mod_card->get_modifier(is_response).get_error(mod_card, origin, origin_card, ctx)) return false;
        }

        if (get_play_card_error(origin, origin_card, ctx)) {
            return false;
        }

        if (filters::is_equip_card(origin_card)) {
            if (is_response || get_all_equip_targets(origin, origin_card, ctx).empty()) {
                return false;
            }
        } else {
            const auto &effects = origin_card->get_effect_list(is_response);
            if (effects.empty() || !rn::all_of(effects, [&](const effect_holder &effect) {
                return play_dispatch::possible(origin, origin_card, effect, ctx);
            })) {
                return false;
            }

            if (const auto &mth = origin_card->get_mth(is_response)) {
                target_list targets;
                targets.reserve(mth.args.size());
                if (!is_possible_mth(origin, origin_card, mth, effects, {}, targets)) {
                    return false;
                }
            }

            if (const modifier_holder &modifier = origin_card->get_modifier(is_response)) {
                auto modifiers_copy = modifiers;
                modifiers_copy.push_back(origin_card);
                auto ctx_copy = ctx;
                modifier.add_context(origin_card, origin, ctx_copy);
                
                return map_cards_playable_with_modifiers(origin, modifiers_copy, is_response, ctx_copy, std::not_fn(rn::empty));
            }
        }
        
        return origin->m_gold >= filters::get_card_cost(origin_card, is_response, ctx);
    }

    static void collect_playable_cards(
        playable_cards_list &result, card_list &modifiers,
        player *origin, card *origin_card, bool is_response, effect_context ctx
    ) {
        const modifier_holder &modifier = origin_card->get_modifier(is_response);
        if (filters::is_equip_card(origin_card) || !modifier) {
            if (modifiers.empty()) {
                result.emplace_back(origin_card);
            } else {
                result.emplace_back(origin_card, modifiers, ctx);
            }
        } else {
            modifier.add_context(origin_card, origin, ctx);

            modifiers.push_back(origin_card);
            map_cards_playable_with_modifiers(origin, modifiers, is_response, ctx, [&](auto &&range) {
                for (card *target_card : range) {
                    collect_playable_cards(result, modifiers, origin, target_card, is_response, ctx);
                }
            });
            modifiers.pop_back();
        }
    }

    playable_cards_list generate_playable_cards_list(player *origin, bool is_response) {
        playable_cards_list result;
        card_list modifiers;

        if (origin) {
            for (card *origin_card : get_all_playable_cards(origin, is_response)) {
                collect_playable_cards(result, modifiers, origin, origin_card, is_response, {});
            }
        }

        return result;
    }
}