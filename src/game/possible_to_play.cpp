#include "possible_to_play.h"

#include "cards/effect_enums.h"
#include "cards/filters.h"

namespace banggame {

    static auto get_all_active_cards(player *origin) {
        return ranges::views::concat(
            origin->m_hand,
            origin->m_table | ranges::views::remove_if(&card::inactive),
            origin->m_characters,
            origin->m_game->m_button_row,
            origin->m_game->m_hidden_deck,
            origin->m_game->m_shop_selection,
            origin->m_game->m_stations,
            origin->m_game->m_train,
            origin->m_game->m_scenario_cards | ranges::views::take_last(1),
            origin->m_game->m_wws_scenario_cards | ranges::views::take_last(1)
        );
    }

    ranges::any_view<card *> get_all_playable_cards(player *origin, bool is_response) {
        return get_all_active_cards(origin)
            | ranges::views::filter([=](card *origin_card) {
                return is_possible_to_play(origin, origin_card, is_response);
            });
    }

    ranges::any_view<player *> make_equip_set(player *origin, card *origin_card, const effect_context &ctx) {
        return origin->m_game->m_players
            | ranges::views::filter([=](player *target) {
                return !get_equip_error(origin, origin_card, target, ctx);
            });
    }

    ranges::any_view<player *> make_player_target_set(player *origin, card *origin_card, const effect_holder &holder, const effect_context &ctx) {
        return origin->m_game->m_players
            | ranges::views::filter([=](player *target) {
                return !filters::check_player_filter(origin, holder.player_filter, target, ctx)
                    && !holder.get_error(origin_card, origin, target, ctx);
            });
    }

    ranges::any_view<card *> make_card_target_set(player *origin, card *origin_card, const effect_holder &holder, const effect_context &ctx) {
        return ranges::views::concat(
            make_player_target_set(origin, origin_card, holder)
            | ranges::views::for_each([](player *target) {
                return ranges::views::concat(
                    target->m_hand,
                    target->m_table,
                    target->m_characters | ranges::views::take(1)
                );
            }),
            origin->m_game->m_selection)
            | ranges::views::filter([=](card *target_card) {
                return !filters::check_card_filter(origin_card, origin, holder.card_filter, target_card, ctx)
                    && !holder.get_error(origin_card, origin, target_card, ctx);
            });
    }

    template<typename T, typename ... Ts>
    inline std::vector<T> vector_concat(const std::vector<T> &vec, Ts && ... args) {
        auto copy = vec;
        copy.emplace_back(FWD(args) ... );
        return copy;
    }

    static bool is_possible_mth_impl(player *origin, card *origin_card, const mth_holder &mth, const effect_list &effects, effect_list::const_iterator effect_it, const effect_context &ctx, const effect_target_list &targets) {
        effect_it = std::ranges::find(effect_it, effects.end(), effect_type::mth_add, &effect_holder::type);
        if (effect_it == effects.end()) {
            return !mth.get_error(origin_card, origin, targets, ctx);
        } else {
            switch (effect_it->target) {
            case target_type::none:
                return is_possible_mth_impl(origin, origin_card, mth, effects, std::next(effect_it), ctx,
                    vector_concat(targets, *effect_it, play_card_target{enums::enum_tag<target_type::none>}));
            case target_type::player:
                return std::ranges::any_of(make_player_target_set(origin, origin_card, *effect_it, ctx), [&](player *target) {
                    return is_possible_mth_impl(origin, origin_card, mth, effects, std::next(effect_it), ctx,
                        vector_concat(targets, *effect_it, play_card_target{enums::enum_tag<target_type::player>, target}));
                });
            case target_type::card:
                return std::ranges::any_of(make_card_target_set(origin, origin_card, *effect_it, ctx), [&](card *target) {
                    return is_possible_mth_impl(origin, origin_card, mth, effects, std::next(effect_it), ctx,
                        vector_concat(targets, *effect_it, play_card_target{enums::enum_tag<target_type::card>, target}));
                });
            default:
                // ignore other target types
                return true;
            }
        }
    }

    static bool is_possible_mth(player *origin, card *origin_card, bool is_response, const effect_context &ctx) {
        const auto &effects = origin_card->get_effect_list(is_response);
        const auto &mth = origin_card->get_mth(is_response);
        return is_possible_mth_impl(origin, origin_card, mth, effects, effects.begin(), ctx, {});
    }

    bool is_possible_to_play_effects(player *origin, card *origin_card, const effect_list &effects, const effect_context &ctx) {
        return !effects.empty() && std::ranges::all_of(effects, [&](const effect_holder &holder) {
            switch (holder.target) {
            case target_type::none:
                return !holder.get_error(origin_card, origin, ctx);
            case target_type::player:
                return contains_at_least(make_player_target_set(origin, origin_card, holder, ctx), 1);
            case target_type::card:
                return contains_at_least(make_card_target_set(origin, origin_card, holder, ctx), 1);
            case target_type::extra_card:
                return ctx.repeat_card || contains_at_least(make_card_target_set(origin, origin_card, holder, ctx), 1);
            case target_type::cards:
                return contains_at_least(make_card_target_set(origin, origin_card, holder, ctx), std::max<int>(1, holder.target_value));
            case target_type::select_cubes:
                return origin->count_cubes() >= holder.target_value;
            case target_type::self_cubes:
                return origin_card->num_cubes >= holder.target_value;
            default:
                return true;
            }
        });
    }

    static ranges::any_view<card *> cards_playable_with_modifiers(player *origin, const std::vector<card *> &modifiers, bool is_response, const effect_context &ctx) {
        auto filter = ranges::views::filter([=](card *origin_card) {
            return is_possible_to_play(origin, origin_card, is_response, modifiers, ctx);
        });
        if (ctx.card_choice) {
            return origin->m_game->m_hidden_deck | filter;
        } else if (ctx.traincost) {
            return origin->m_game->m_train | filter;
        } else if (ctx.repeat_card) {
            return ranges::views::single(ctx.repeat_card) | filter;
        } else {
            return get_all_active_cards(origin) | filter;
        }
    }

    bool is_possible_to_play(player *origin, card *origin_card, bool is_response, const std::vector<card *> &modifiers, const effect_context &ctx) {
        for (card *mod_card : modifiers) {
            if (mod_card == origin_card) return false;
            if (mod_card->modifier.get_error(mod_card, origin, origin_card, ctx)) return false;
        }

        if (filters::is_equip_card(origin_card)) {
            return !is_response
                && contains_at_least(make_equip_set(origin, origin_card, ctx), 1)
                && origin->m_gold >= filters::get_card_cost(origin_card, is_response, ctx);
        }
        
        if (origin->get_play_card_error(origin_card, ctx)
            || origin->m_game->is_disabled(origin_card)
            || !is_possible_to_play_effects(origin, origin_card, origin_card->get_effect_list(is_response), ctx)
            || !is_possible_mth(origin, origin_card, is_response, ctx))
        {
            return false;
        }
        
        if (origin_card->is_modifier()) {
            auto ctx_copy = ctx;
            origin_card->modifier.add_context(origin_card, origin, ctx_copy);
            
            return contains_at_least(cards_playable_with_modifiers(origin, vector_concat(modifiers, origin_card), is_response, ctx_copy), 1);
        } else {
            return origin->m_gold >= filters::get_card_cost(origin_card, is_response, ctx);
        }
    }

    static card_modifier_node generate_card_modifier_node(player *origin, card *origin_card, bool is_response, const std::vector<card *> &modifiers, const effect_context &ctx) {
        card_modifier_node node { .card = origin_card };
        if (!filters::is_equip_card(origin_card) && origin_card->is_modifier()) {
            std::vector<card *> modifiers_copy = vector_concat(modifiers, origin_card);
            effect_context ctx_copy = ctx;
            origin_card->modifier.add_context(origin_card, origin, ctx_copy);

            for (card *target_card : cards_playable_with_modifiers(origin, modifiers_copy, is_response, ctx_copy)) {
                node.branches.push_back(generate_card_modifier_node(origin, target_card, is_response, modifiers_copy, ctx_copy));
            }
        }
        return node;
    }

    card_modifier_tree generate_card_modifier_tree(player *origin, bool is_response) {
        card_modifier_tree tree;
        for (card *origin_card : get_all_playable_cards(origin, is_response)) {
            tree.push_back(generate_card_modifier_node(origin, origin_card, is_response, {}, {}));
        }
        return tree;
    }
}