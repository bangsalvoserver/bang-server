#include "possible_to_play.h"

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

    static auto get_all_targetable_cards(player *origin) {
        return rv::concat(
            origin->m_game->m_players | rv::for_each([](const player *p) {
                return rv::concat(p->m_hand, p->m_table, p->m_characters);
            }),
            origin->m_game->m_selection,
            origin->m_game->m_deck | rv::take(1),
            origin->m_game->m_discards | rv::take(1)
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

    rn::any_view<card *> make_card_target_set(player *origin, card *origin_card, const effect_holder &holder, const effect_context &ctx) {
        return get_all_targetable_cards(origin)
            | rv::filter([=](card *target_card) {
                return (!target_card->owner || !filters::check_player_filter(origin, holder.player_filter, target_card->owner, ctx))
                    && !filters::check_card_filter(origin_card, origin, holder.card_filter, target_card, ctx)
                    && !holder.get_error(origin_card, origin, target_card, ctx);
            });
    }

    rn::any_view<player *> get_request_target_set_players(player *origin) {
        if (origin) {
            if (auto req = origin->m_game->top_request<interface_target_set_players>(origin)) {
                return origin->m_game->m_players
                    | rv::filter([=](const player *p) {
                        return req->in_target_set(p);
                    });
            }
        }
        return rv::empty<player *>;
    }

    rn::any_view<card *> get_request_target_set_cards(player *origin) {
        if (origin) {
            if (auto req = origin->m_game->top_request<interface_target_set_cards>(origin)) {
                return get_all_targetable_cards(origin)
                    | rv::filter([=](const card *target_card) {
                        return req->in_target_set(target_card);
                    });
            }
        }
        return rv::empty<card *>;
    }

    static bool is_possible_mth(player *origin, card *origin_card, const mth_holder &mth, const effect_list &effects, const effect_context &ctx, target_list &targets) {
        if (targets.size() == mth.args.size()) {
            return !mth_holder{
                mth.type,
                serial::int_list(small_int_set_sized_tag, targets.size())
            }.get_error(origin_card, origin, targets, ctx);
        }
        const auto &effect = effects.at(mth.args[targets.size()]);
        if (effect.target == TARGET_TYPE(player)) {
            for (player *target : make_player_target_set(origin, origin_card, effect, ctx)) {
                targets.emplace_back(utils::tag<"player">{}, target);
                bool result = is_possible_mth(origin, origin_card, mth, effects, ctx, targets);
                targets.pop_back();
                if (result) return true;
            }
            return false;
        } else if (effect.target == TARGET_TYPE(card)) {
            for (card *target : make_card_target_set(origin, origin_card, effect, ctx)) {
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
            if (mod_card->get_modifier(is_response).get_error(mod_card, origin, origin_card, ctx)) return false;
        }

        if (get_play_card_error(origin, origin_card, ctx)) {
            return false;
        }

        if (filters::is_equip_card(origin_card)) {
            if (is_response || !contains_at_least(make_equip_set(origin, origin_card, ctx), 1)) {
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
                
                return contains_at_least(cards_playable_with_modifiers(origin, modifiers_copy, is_response, ctx_copy), 1);
            }
        }
        
        return origin->m_gold >= filters::get_card_cost(origin_card, is_response, ctx);
    }

    static void collect_playable_cards(
        playable_cards_list &result, std::vector<card *> &modifiers,
        player *origin, card *origin_card, bool is_response,
        const effect_context &ctx = {}
    ) {
        const modifier_holder &modifier = origin_card->get_modifier(is_response);
        if (filters::is_equip_card(origin_card) || !modifier) {
            if (modifiers.empty()) {
                result.emplace_back(origin_card);
            } else {
                result.emplace_back(origin_card, modifiers | rn::to<serial::card_list>, ctx);
            }
        } else {
            auto ctx_copy = ctx;
            modifier.add_context(origin_card, origin, ctx_copy);

            modifiers.push_back(origin_card);
            for (card *target_card : cards_playable_with_modifiers(origin, modifiers, is_response, ctx_copy)) {
                collect_playable_cards(result, modifiers, origin, target_card, is_response, ctx_copy);
            }
            modifiers.pop_back();
        }
    }

    playable_cards_list generate_playable_cards_list(player *origin, bool is_response) {
        playable_cards_list result;
        std::vector<card *> modifiers;

        if (origin) {
            for (card *origin_card : get_all_playable_cards(origin, is_response)) {
                collect_playable_cards(result, modifiers, origin, origin_card, is_response);
            }
        }

        return result;
    }
}