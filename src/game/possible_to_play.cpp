#include "possible_to_play.h"

#include "cards/effect_enums.h"
#include "cards/filters.h"

#include "play_verify.h"

namespace banggame {

    static auto get_all_active_cards(player *origin) {
        return rv::concat(
            origin->m_hand,
            origin->m_table,
            origin->m_characters,
            origin->m_game->m_button_row,
            origin->m_game->m_hidden_deck,
            origin->m_game->m_shop_selection,
            origin->m_game->m_stations,
            origin->m_game->m_train,
            origin->m_game->m_scenario_cards | rv::take_last(1),
            origin->m_game->m_wws_scenario_cards | rv::take_last(1)
        );
    }

    rn::any_view<card *> get_all_playable_cards(player *origin, bool is_response) {
        return get_all_active_cards(origin)
            | rv::filter([=](card *origin_card) {
                return is_possible_to_play(origin, origin_card, is_response);
            });
    }

    rn::any_view<player *> make_equip_set(player *origin, card *origin_card, const effect_context &ctx) {
        return origin->m_game->m_players
            | rv::filter([=](player *target) {
                return !get_equip_error(origin, origin_card, target, ctx);
            });
    }

    rn::any_view<player *> make_player_target_set(player *origin, card *origin_card, const effect_holder &holder, const effect_context &ctx) {
        return origin->m_game->m_players
            | rv::filter([=](player *target) {
                return !filters::check_player_filter(origin, holder.player_filter, target, ctx)
                    && !holder.get_error(origin_card, origin, target, ctx);
            });
    }

    rn::any_view<std::pair<player *, player *>> make_adjacent_players_target_set(player *origin, card *origin_card, const effect_context &ctx) {
        effect_holder effect1 { .player_filter = target_player_filter::notself | target_player_filter::reachable };
        effect_holder effect2 { .player_filter = target_player_filter::notself };
        return make_player_target_set(origin, origin_card, effect1, ctx) | rv::for_each([=](player *target1) {
            return make_player_target_set(origin, origin_card, effect2, ctx) | rv::transform([=](player *target2) {
                return std::pair{target1, target2};
            });
        })
        | rv::filter([=](const auto &targets) {
            auto [target1, target2] = targets;
            return origin->m_game->calc_distance(target1, target2) == 1;
        });
    }

    rn::any_view<card *> make_card_target_set(player *origin, card *origin_card, const effect_holder &holder, const effect_context &ctx) {
        return rv::concat(
            make_player_target_set(origin, origin_card, holder)
            | rv::for_each([](player *target) {
                return rv::concat(
                    target->m_hand,
                    target->m_table,
                    target->m_characters | rv::take(1)
                );
            }),
            origin->m_game->m_selection)
            | rv::filter([=](card *target_card) {
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
        effect_it = rn::find(effect_it, effects.end(), effect_type::mth_add, &effect_holder::type);
        if (effect_it == effects.end()) {
            return !mth.get_error(origin_card, origin, targets, ctx);
        } else {
            switch (effect_it->target) {
            case target_type::none:
                return is_possible_mth_impl(origin, origin_card, mth, effects, std::next(effect_it), ctx,
                    vector_concat(targets, *effect_it, play_card_target{enums::enum_tag<target_type::none>}));
            case target_type::player:
                return rn::any_of(make_player_target_set(origin, origin_card, *effect_it, ctx), [&](player *target) {
                    return is_possible_mth_impl(origin, origin_card, mth, effects, std::next(effect_it), ctx,
                        vector_concat(targets, *effect_it, play_card_target{enums::enum_tag<target_type::player>, target}));
                });
            case target_type::card:
                return rn::any_of(make_card_target_set(origin, origin_card, *effect_it, ctx), [&](card *target) {
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
        return is_possible_to_play_effects(origin, origin_card, effects, ctx)
            && is_possible_mth_impl(origin, origin_card, mth, effects, effects.begin(), ctx, {});
    }

    bool is_possible_to_play_effects(player *origin, card *origin_card, const effect_list &effects, const effect_context &ctx) {
        return !effects.empty() && rn::all_of(effects, [&](const effect_holder &holder) {
            switch (holder.target) {
            case target_type::none:
                return !holder.get_error(origin_card, origin, ctx);
            case target_type::player:
                return contains_at_least(make_player_target_set(origin, origin_card, holder, ctx), 1);
            case target_type::adjacent_players:
                return contains_at_least(make_adjacent_players_target_set(origin, origin_card, ctx), 1);
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

    static rn::any_view<card *> cards_playable_with_modifiers(player *origin, const std::vector<card *> &modifiers, bool is_response, const effect_context &ctx) {
        auto filter = rv::filter([=](card *origin_card) {
            return is_possible_to_play(origin, origin_card, is_response, modifiers, ctx);
        });
        if (ctx.card_choice) {
            return origin->m_game->m_hidden_deck | filter;
        } else if (ctx.traincost) {
            return origin->m_game->m_train | filter;
        } else if (ctx.repeat_card) {
            return rv::single(ctx.repeat_card) | filter;
        } else {
            return get_all_active_cards(origin) | filter;
        }
    }

    bool is_possible_to_play(player *origin, card *origin_card, bool is_response, const std::vector<card *> &modifiers, const effect_context &ctx) {
        for (card *mod_card : modifiers) {
            if (mod_card == origin_card) return false;
            if (mod_card->modifier.get_error(mod_card, origin, origin_card, ctx)) return false;
        }

        if (get_play_card_error(origin, origin_card, ctx)) {
            return false;
        }

        if (filters::is_equip_card(origin_card)) {
            if (is_response || !contains_at_least(make_equip_set(origin, origin_card, ctx), 1)) {
                return false;
            }
        } else {
            if (!is_possible_mth(origin, origin_card, is_response, ctx)) {
                return false;
            }
            
            if (origin_card->is_modifier()) {
                auto ctx_copy = ctx;
                origin_card->modifier.add_context(origin_card, origin, ctx_copy);
                
                return contains_at_least(cards_playable_with_modifiers(origin, vector_concat(modifiers, origin_card), is_response, ctx_copy), 1);
            }
        }
        
        return origin->m_gold >= filters::get_card_cost(origin_card, is_response, ctx);
    }

    static card_modifier_node generate_card_modifier_node(player *origin, card *origin_card, bool is_response, const std::vector<card *> &modifiers, const effect_context &ctx) {
        card_modifier_node node { .card = origin_card };
        if (!filters::is_equip_card(origin_card) && origin_card->is_modifier()) {
            std::vector<card *> modifiers_copy = vector_concat(modifiers, origin_card);
            auto ctx_copy = ctx;
            origin_card->modifier.add_context(origin_card, origin, ctx_copy);

            for (card *target_card : cards_playable_with_modifiers(origin, modifiers_copy, is_response, ctx_copy)) {
                node.branches.push_back(generate_card_modifier_node(origin, target_card, is_response, modifiers_copy, ctx_copy));
            }
        }
        return node;
    }

    card_modifier_tree generate_card_modifier_tree(player *origin, bool is_response) {
        card_modifier_tree tree;
        if (origin) {
            for (card *origin_card : get_all_playable_cards(origin, is_response)) {
                tree.push_back(generate_card_modifier_node(origin, origin_card, is_response, {}, {}));
            }
        }
        return tree;
    }

    rn::any_view<card *> get_pick_cards(player *origin) {
        if (origin) {
            if (auto req = origin->m_game->top_request<request_picking_base>(origin)) {
                return rv::concat(
                    origin->m_game->m_players | rv::for_each([](player *p) {
                        return rv::concat(p->m_hand, p->m_table, p->m_characters);
                    }),
                    origin->m_game->m_selection,
                    origin->m_game->m_deck | rv::take(1),
                    origin->m_game->m_discards | rv::take(1)
                )
                | rv::filter([=](card *target_card) {
                    return req->can_pick(target_card);
                });
            }
        }
        return rv::empty<card *>;
    }
}