#include "possible_to_play.h"

#include "filters.h"

namespace banggame {

    static auto get_all_active_cards(player *origin, bool include_last) {
        return ranges::views::concat(
            origin->m_hand,
            origin->m_table | ranges::views::remove_if(&card::inactive),
            origin->m_characters,
            origin->m_game->m_button_row,
            origin->m_game->m_hidden_deck,
            origin->m_game->m_shop_selection,
            origin->m_game->m_scenario_cards | ranges::views::take_last(1),
            origin->m_game->m_wws_scenario_cards | ranges::views::take_last(1),
            ranges::views::single(origin->get_last_played_card())
                | ranges::views::filter([=](card *last_played_card) {
                    return last_played_card && include_last;
                })
        );
    }

    ranges::any_view<card *> get_all_playable_cards(player *origin, bool is_response) {
        return get_all_active_cards(origin, false)
            | ranges::views::filter([=](card *origin_card) {
                return is_possible_to_play(origin, origin_card, is_response);
            });
    }

    ranges::any_view<player *> make_equip_set(player *origin, card *origin_card) {
        return origin->m_game->m_players
            | ranges::views::filter([=](player *target) {
                return !get_equip_error(origin, origin_card, target);
            });
    }

    ranges::any_view<player *> make_player_target_set(player *origin, card *origin_card, const effect_holder &holder, const effect_context &ctx) {
        return origin->m_game->m_players
            | ranges::views::filter([=](player *target) {
                return !check_player_filter(origin, holder.player_filter, target, ctx)
                    && !holder.get_error(origin_card, origin, target, ctx);
            });
    }

    ranges::any_view<card *> make_card_target_set(player *origin, card *origin_card, const effect_holder &holder, const effect_context &ctx) {
        return make_player_target_set(origin, origin_card, holder)
            | ranges::views::for_each([](player *target) {
                return ranges::views::concat(
                    target->m_hand,
                    target->m_table,
                    target->m_characters | ranges::views::take(1)
                );
            })
            | ranges::views::filter([=](card *target_card) {
                return !check_card_filter(origin_card, origin, holder.card_filter, target_card, ctx)
                    && !holder.get_error(origin_card, origin, target_card, ctx);
            });
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
                return ctx.repeating || contains_at_least(make_card_target_set(origin, origin_card, holder, ctx), 1);
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

    ranges::any_view<card *> cards_playable_with_modifiers(player *origin, const std::vector<card *> &modifiers, bool is_response, const effect_context &ctx) {
        return get_all_active_cards(origin, true) | ranges::views::filter([=](card *origin_card) {
            return is_possible_to_play(origin, origin_card, is_response, modifiers, ctx);
        });
    }

    bool is_possible_to_play(player *origin, card *origin_card, bool is_response, const std::vector<card *> &modifiers, const effect_context &ctx) {
        if (std::ranges::any_of(modifiers, [&](card *mod_card) {
            return origin_card == mod_card || !allowed_card_with_modifier(origin, mod_card, origin_card);
        })) {
            return false;
        } else if ((origin_card->pocket == pocket_type::player_hand || origin_card->pocket == pocket_type::shop_selection) && !origin_card->is_brown()) {
            return !is_response
                && contains_at_least(make_equip_set(origin, origin_card), 1)
                && origin->m_gold >= get_card_cost(origin_card, is_response, ctx);
        } else if (origin->get_play_card_error(origin_card) || origin->m_game->is_disabled(origin_card)) {
            return false;
        } else if (!is_possible_to_play_effects(origin, origin_card, origin_card->get_effect_list(is_response), ctx)) {
            return false;
        } else if (origin_card->is_modifier()) {
            if (!std::transform_reduce(
                modifiers.begin(), modifiers.end(), modifier_bitset(origin_card->modifier_type()), std::bit_and(),
                [](card *mod) { return allowed_modifiers_after(mod->modifier_type()); }
            )) {
                return false;
            }

            auto ctx_copy = ctx;
            origin_card->modifier.add_context(origin_card, origin, ctx_copy);
            
            auto modifiers_copy = modifiers;
            modifiers_copy.push_back(origin_card);
            return contains_at_least(cards_playable_with_modifiers(origin, modifiers_copy, is_response, ctx_copy), 1);
        } else {
            return origin->m_gold >= get_card_cost(origin_card, is_response, ctx);
        }
    }
}