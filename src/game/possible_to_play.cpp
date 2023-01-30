#include "possible_to_play.h"

namespace banggame {

    static const effect_list &get_effect_list(card *origin_card, effect_list_index index) {
        switch (index) {
        case effect_list_index::effects:
        default:
            return origin_card->effects;
        case effect_list_index::responses:
            return origin_card->responses;
        case effect_list_index::optionals:
            return origin_card->optionals;
        }
    }

    ranges::any_view<player *> make_equip_set(player *origin, card *origin_card) {
        if (origin_card->self_equippable()) {
            return ranges::views::single(origin);
        }
        return origin->m_game->m_players
            | ranges::views::filter([=](player *target) {
                return !check_player_filter(origin, origin_card->equip_target, target)
                    && !target->find_equipped_card(origin_card);
            });
    }

    ranges::any_view<player *> make_player_target_set(player *origin, card *origin_card, const effect_holder &holder, const effect_context &ctx) {
        return origin->m_game->m_players
            | ranges::views::filter([=](player *target) {
                return !check_player_filter(origin, holder.player_filter, target, ctx)
                    && !holder.verify(origin_card, origin, target, ctx);
            });
    }

    ranges::any_view<card *> make_card_target_set(player *origin, card *origin_card, const effect_holder &holder, const effect_context &ctx) {
        return make_player_target_set(origin, origin_card, holder)
            | ranges::views::for_each([](player *target) {
                return ranges::views::concat(target->m_hand, target->m_table);
            })
            | ranges::views::filter([=](card *target_card) {
                return !check_card_filter(origin_card, origin, holder.card_filter, target_card, ctx)
                    && !holder.verify(origin_card, origin, target_card, ctx);
            });
    }

    static bool is_possible_to_play_impl(player *origin, card *origin_card, effect_list_index index, const effect_context &ctx) {
        return std::ranges::all_of(get_effect_list(origin_card, index), [&](const effect_holder &holder) {
            switch (holder.target) {
            case target_type::none:
                return !holder.verify(origin_card, origin, ctx);
            case target_type::player:
                return contains_at_least(make_player_target_set(origin, origin_card, holder, ctx), 1);
            case target_type::card:
            case target_type::extra_card:
                return contains_at_least(make_card_target_set(origin, origin_card, holder, ctx), 1);
            case target_type::cards:
                return contains_at_least(make_card_target_set(origin, origin_card, holder, ctx), std::max<int>(1, holder.target_value));
            case target_type::select_cubes:
                return origin->count_cubes() >= holder.target_value;
            case target_type::self_cubes:
                return origin_card->num_cubes >= holder.target_value;
            case target_type::fanning_targets:
                return std::ranges::any_of(origin->m_game->m_players, [&](player *target) {
                    if (target != origin && (ctx.ignore_distances || origin->m_game->calc_distance(origin, target) <= origin->m_weapon_range + origin->m_range_mod)) {
                        if (player *target2 = *std::next(player_iterator(target)); target2 != origin && target2->m_distance_mod == 0) return true;
                        if (player *target2 = *std::prev(player_iterator(target)); target2 != origin && target2->m_distance_mod == 0) return true;
                    }
                    return false;
                });
            default:
                return true;
            }
        });
    }

    static bool any_card_playable_with_modifiers(player *origin, const std::vector<card *> &modifiers, effect_list_index index, const effect_context &ctx) {
        return ranges::any_of(ranges::views::concat(
            origin->m_characters,
            origin->m_table | ranges::views::remove_if(&card::inactive),
            origin->m_hand | ranges::views::filter(&card::is_brown),
            origin->m_game->m_shop_selection,
            origin->m_game->m_hidden_deck,
            origin->m_game->m_scenario_cards | ranges::views::take_last(1),
            origin->m_game->m_wws_scenario_cards | ranges::views::take_last(1)
        ),
        [&](card *target_card) {
            if (ranges::contains(modifiers, target_card)) return false;

            if (!std::ranges::all_of(modifiers, [&](card *mod) {
                return allowed_card_with_modifier(origin, mod, target_card);
            })) return false;

            if (!is_possible_to_play_impl(origin, target_card, index, ctx)) return false;

            if (!target_card->is_modifier()) {
                return !get_effect_list(target_card, index).empty();
            } else {
                if (!std::transform_reduce(
                    modifiers.begin(), modifiers.end(), modifier_bitset(target_card->modifier_type()), std::bit_and(),
                    [](card *mod) { return allowed_modifiers_after(mod->modifier_type()); }
                )) {
                    return false;
                }
                auto modifiers_new = modifiers;
                modifiers_new.push_back(target_card);
                return any_card_playable_with_modifiers(origin, modifiers_new, index, ctx);
            }
        });
    }

    bool is_possible_to_play(player *origin, card *origin_card, effect_list_index index, const effect_context &ctx) {
        if (!origin_card->is_modifier()) {
            if (get_effect_list(origin_card, index).empty()) return false;
        } else {
            if (!any_card_playable_with_modifiers(origin, std::vector{origin_card}, index, ctx)) {
                return false;
            }
        }
        return is_possible_to_play_impl(origin, origin_card, index, ctx);
    }
}