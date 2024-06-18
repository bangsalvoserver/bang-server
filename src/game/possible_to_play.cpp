#include "possible_to_play.h"

#include "effects/base/pick.h"

#include "game/filters.h"
#include "cards/filter_enums.h"

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
        if (bool(holder.player_filter & target_player_filter::equip_player)) {
            if (!ctx.playing_card) {
                return rv::single(origin);
            }
            return make_equip_set(origin, ctx.playing_card, ctx);
        }
        return origin->m_game->m_players
            | rv::filter([=](player *target) {
                return !filters::check_player_filter(origin, holder.player_filter, target, ctx)
                    && !holder.type->get_error_player(holder.effect_value, origin_card, origin, target, ctx);
            });
    }

    rn::any_view<card *> make_card_target_set(player *origin, card *origin_card, const effect_holder &holder, const effect_context &ctx) {
        if (bool(holder.card_filter & target_card_filter::equip_card)) {
            return get_all_active_cards(origin) | rv::filter(filters::is_equip_card);
        }
        if (bool(holder.card_filter & target_card_filter::pick_card)) {
            return get_pick_cards(origin);
        }
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
                    && !holder.type->get_error_card(holder.effect_value, origin_card, origin, target_card, ctx);
            });
    }

    static bool is_possible_mth_impl(player *origin, card *origin_card, const mth_holder &mth, const effect_list &effects, const effect_context &ctx, const target_list &targets) {
        if (targets.size() == mth.args.size()) {
            return !mth.type->get_error(origin_card, origin, targets,
                    serial::int_list(small_int_set_sized_tag, targets.size()), ctx);
        }
        const auto &effect = effects.at(mth.args[targets.size()]);
        auto recurse = [&](const play_card_target &target){
            auto ctx_copy = ctx;
            play_dispatch::add_context(origin, origin_card, effect, ctx_copy, target);

            auto targets_copy = targets;
            targets_copy.push_back(target);

            return is_possible_mth_impl(origin, origin_card, mth, effects, ctx_copy, targets_copy);
        };
        switch (effect.target) {
        case target_type::player:
            return rn::any_of(make_player_target_set(origin, origin_card, effect, ctx), [&](player *target) {
                return recurse({enums::enum_tag<target_type::player>, target});
            });
        case target_type::card:
            return rn::any_of(make_card_target_set(origin, origin_card, effect, ctx), [&](card *target) {
                return recurse({enums::enum_tag<target_type::card>, target});
            });
        default:
            // ignore other target types
            return true;
        }
    }

    static bool is_possible_mth(player *origin, card *origin_card, bool is_response, const effect_context &ctx) {
        const auto &effects = origin_card->get_effect_list(is_response);
        if (effects.empty() || !rn::all_of(effects, [&](const effect_holder &effect) {
            return play_dispatch::possible(origin, origin_card, effect, ctx);
        })) {
            return false;
        }

        const auto &mth = origin_card->get_mth(is_response);
        if (mth.type && !is_possible_mth_impl(origin, origin_card, mth, effects, ctx, {})) {
            return false;
        }

        return true;
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
            if (mod_card->get_modifier(is_response).type->get_error(mod_card, origin, origin_card, ctx)) return false;
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

            const modifier_holder &modifier = origin_card->get_modifier(is_response);
            if (modifier.type != nullptr) {
                auto modifiers_copy = modifiers;
                modifiers_copy.push_back(origin_card);
                auto ctx_copy = ctx;
                modifier.type->add_context(origin_card, origin, ctx_copy);
                
                return contains_at_least(cards_playable_with_modifiers(origin, modifiers_copy, is_response, ctx_copy), 1);
            }
        }
        
        return origin->m_gold >= filters::get_card_cost(origin_card, is_response, ctx);
    }

    static card_modifier_node generate_card_modifier_node(player *origin, card *origin_card, bool is_response, const std::vector<card *> &modifiers, const effect_context &ctx) {
        card_modifier_node node { .card = origin_card };
        const modifier_holder &modifier = origin_card->get_modifier(is_response);
        if (!filters::is_equip_card(origin_card) && modifier.type != nullptr) {
            auto modifiers_copy = modifiers;
            modifiers_copy.push_back(origin_card);
            auto ctx_copy = ctx;
            modifier.type->add_context(origin_card, origin, ctx_copy);

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
            if (auto req = origin->m_game->top_request<interface_picking>(origin)) {
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

    rn::any_view<player *> get_request_target_set(player *origin) {
        if (origin) {
            if (auto req = origin->m_game->top_request<interface_target_set>(origin)) {
                return origin->m_game->m_players | rv::filter([=](const player *p) {
                    return req->in_target_set(p);
                });
            }
        }
        return rv::empty<player *>;
    }
}