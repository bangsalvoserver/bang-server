#include "play_verify.h"

#include "play_visitor.h"

#include "cards/base/requests.h"
#include "effect_list_zip.h"

#include "utils/raii_editor.h"
#include "utils/utils.h"

namespace banggame {

    static game_string verify_target_list(player *origin, card *origin_card, bool is_response, const target_list &targets, effect_context &ctx) {
        MAYBE_RETURN(origin->m_game->call_event<event_type::check_play_card>(origin, origin_card, game_string{}));
        
        size_t diff = targets.size() - origin_card->get_effect_list(is_response).size();
        if (auto repeatable = origin_card->get_tag_value(tag_type::repeatable)) {
            if (diff < 0 || diff % origin_card->optionals.size() != 0
                || (*repeatable > 0 && diff > (origin_card->optionals.size() * *repeatable)))
            {
                return "ERROR_INVALID_TARGETS";
            }
        } else if (diff != 0 && diff != origin_card->optionals.size()) {
            return "ERROR_INVALID_TARGETS";
        }

        target_list mth_targets;
        for (const auto &[target, effect] : zip_card_targets(targets, origin_card, is_response)) {
            if (!target.is(effect.target)) {
                return "ERROR_INVALID_TARGET_TYPE";
            } else if (effect.type == effect_type::mth_add) {
                mth_targets.push_back(target);
            } else if (effect.type == effect_type::ctx_add) {
                if (target.is(target_type::card)) {
                    origin_card->modifier.add_context(origin_card, origin, target.get<target_type::card>(), ctx);
                } else if (target.is(target_type::player)) {
                    origin_card->modifier.add_context(origin_card, origin, target.get<target_type::player>(), ctx);
                } else {
                    return "ERROR_INVALID_TARGET_TYPE";
                }
            }
            
            MAYBE_RETURN(enums::visit_indexed(
                [&]<target_type E>(enums::enum_tag_t<E>, auto && ... args) -> game_string {
                    return play_visitor<E>{origin, origin_card, effect}.get_error(ctx, FWD(args) ...);
                }, target));
        }

        return origin_card->get_mth(is_response).get_error(origin_card, origin, mth_targets, ctx);
    }
    
    static game_string verify_modifiers(player *origin, card *origin_card, bool is_response, const modifier_list &modifiers, effect_context &ctx) {
        auto allowed_modifiers = allowed_modifiers_after(card_modifier_type::none);
        for (const auto &[mod_card, targets] : modifiers) {
            if (!mod_card->is_modifier()) {
                return "ERROR_CARD_IS_NOT_MODIFIER";
            }
            if (card *disabler = origin->m_game->get_disabler(mod_card)) {
                return {"ERROR_CARD_DISABLED_BY", mod_card.get(), disabler};
            }
            if (!bool(allowed_modifiers & modifier_bitset(mod_card->modifier_type()))) {
                return "ERROR_INVALID_MODIFIER_CARD";
            }
            if (!allowed_card_with_modifier(origin, mod_card, origin_card)) {
                return "ERROR_INVALID_CARD_WITH_MODIFIER";
            }
            mod_card->modifier.add_context(mod_card, origin, ctx);
            MAYBE_RETURN(verify_target_list(origin, mod_card, is_response, targets, ctx));
            allowed_modifiers &= allowed_modifiers_after(mod_card->modifier_type());
        }
        return {};
    }

    static game_string verify_duplicates(player *origin, card *origin_card, bool is_response, const target_list &targets, const modifier_list &modifiers) {
        struct {
            std::set<card *> selected_cards;
            std::set<player *> selected_players;
            card_cube_count selected_cubes;

            game_string operator()(player *origin, card *origin_card, const effect_holder &effect, const play_card_target &target) {
                return enums::visit_indexed([&]<target_type E>(enums::enum_tag_t<E>, auto && ... args) -> game_string {
                    auto [players, cards, cubes] = play_visitor<E>{origin, origin_card, effect}.duplicates(FWD(args) ... );
                    selected_players.merge(players);
                    if (!players.empty()) {
                        return {"ERROR_DUPLICATE_PLAYER", *players.begin()};
                    }
                    selected_cards.merge(cards);
                    if (!cards.empty()) {
                        return {"ERROR_DUPLICATE_CARD", *cards.begin()};
                    }
                    for (auto &[card, ncubes] : cubes) {
                        if (selected_cubes[card] += ncubes > card->num_cubes) {
                            return {"ERROR_NOT_ENOUGH_CUBES_ON", card};
                        }
                    }
                    return {};
                }, target);
            }
        } check;

        for (const auto &[mod_card, mod_targets] : modifiers) {
            if (!check.selected_cards.emplace(mod_card).second) {
                return {"ERROR_DUPLICATE_CARD", mod_card.get()};
            }
            for (const auto &[target, effect] : zip_card_targets(mod_targets, mod_card, is_response)) {
                MAYBE_RETURN(check(origin, mod_card, effect, target));
            }
        }

        for (const auto &[target, effect] : zip_card_targets(targets, origin_card, is_response)) {
            MAYBE_RETURN(check(origin, origin_card, effect, target));
        }

        return {};
    }

    static game_string verify_equip_target(player *origin, card *origin_card, const target_list &targets) {
        if (card *disabler = origin->m_game->get_disabler(origin_card)) {
            return {"ERROR_CARD_DISABLED_BY", origin_card, disabler};
        }
        if (origin->m_game->check_flags(game_flags::disable_equipping)) {
            return "ERROR_CANT_EQUIP_CARDS";
        }
        MAYBE_RETURN(origin->m_game->call_event<event_type::check_play_card>(origin, origin_card, game_string{}));
        player *target = origin;
        if (!origin_card->self_equippable()) {
            if (targets.size() != 1 || !targets.front().is(target_type::player)) {
                return "ERROR_INVALID_EQUIP_TARGET";
            }
            target = targets.front().get<target_type::player>();
            MAYBE_RETURN(check_player_filter(origin, origin_card->equip_target, target));
        }
        if (card *equipped = target->find_equipped_card(origin_card)) {
            return {"ERROR_DUPLICATED_CARD", equipped};
        }
        if (origin_card->is_orange() && origin->m_game->num_cubes < 3) {
            return "ERROR_NOT_ENOUGH_CUBES";
        }
        return {};
    }

    static game_string verify_card_targets(player *origin, card *origin_card, bool is_response, const target_list &targets, const modifier_list &modifiers, effect_context &ctx) {
        if (origin_card->is_modifier()) {
            return "ERROR_CARD_IS_MODIFIER";
        }
        if (origin_card->get_effect_list(is_response).empty()) {
            return "ERROR_EFFECT_LIST_EMPTY";
        }
        if (card *disabler = origin->m_game->get_disabler(origin_card)) {
            return {"ERROR_CARD_DISABLED_BY", origin_card, disabler};
        }
        if (origin_card->inactive) {
            return {"ERROR_CARD_INACTIVE", origin_card};
        }
        MAYBE_RETURN(verify_modifiers(origin, origin_card, is_response, modifiers, ctx));
        MAYBE_RETURN(verify_target_list(origin, origin_card, is_response, targets, ctx));
        MAYBE_RETURN(verify_duplicates(origin, origin_card, is_response, targets, modifiers));

        switch (origin_card->pocket) {
        case pocket_type::scenario_card:
            if (origin_card != origin->m_game->m_scenario_cards.back()) {
                return "ERROR_INVALID_SCENARIO_CARD";
            }
            break;
        case pocket_type::wws_scenario_card:
            if (origin_card != origin->m_game->m_wws_scenario_cards.back()) {
                return "ERROR_INVALID_SCENARIO_CARD";
            }
            break;
        case pocket_type::main_deck:
        case pocket_type::discard_pile:
        case pocket_type::shop_discard:
            if (!ctx.repeating) {
                return "ERROR_INVALID_CARD_POCKET";
            }
            break;
        }

        return {};
    }

    static game_string check_prompt(player *origin, card *origin_card, bool is_response, const target_list &targets, const modifier_list &modifiers, const effect_context &ctx) {
        auto check = [&](card *c, const target_list &ts) {
            target_list mth_targets;
            for (const auto &[target, effect] : zip_card_targets(ts, c, is_response)) {
                if (effect.type == effect_type::mth_add) {
                    mth_targets.push_back(target);
                }
                MAYBE_RETURN(enums::visit_indexed(
                    [&]<target_type E>(enums::enum_tag_t<E>, auto && ... args) {
                        return play_visitor<E>{origin, c, effect}.prompt(ctx, FWD(args) ... );
                    }, target));
            }

            return c->get_mth(is_response).on_prompt(c, origin, mth_targets, ctx);
        };

        for (const auto &[mod_card, mod_targets] : modifiers) {
            MAYBE_RETURN(mod_card->modifier.on_prompt(mod_card, origin, origin_card));
            MAYBE_RETURN(check(mod_card, mod_targets));
        }

        return check(origin_card, targets);
    }

    static game_string check_prompt_equip(card *origin_card, player *origin, player *target) {
        for (const auto &e : origin_card->equips) {
            if (auto prompt_message = e.on_prompt(origin_card, origin, target)) {
                return prompt_message;
            }
        }
        return {};
    }

    inline std::vector<card *> get_modifier_cards(const modifier_list &modifiers) {
        return modifiers
            | ranges::views::transform(&modifier_pair::card)
            | ranges::to<std::vector<card *>>;
    }

    static void log_played_card(card *origin_card, player *origin, bool is_response) {
        if (origin_card->has_tag(tag_type::skip_logs)) return;
        
        switch (origin_card->pocket) {
        case pocket_type::player_hand:
        case pocket_type::scenario_card:
            origin->m_game->add_log(is_response ? "LOG_RESPONDED_WITH_CARD" : "LOG_PLAYED_CARD", origin_card, origin);
            break;
        case pocket_type::player_table:
            origin->m_game->add_log(is_response ? "LOG_RESPONDED_WITH_CARD" : "LOG_PLAYED_TABLE_CARD", origin_card, origin);
            break;
        case pocket_type::player_character:
            origin->m_game->add_log(is_response ?
                origin_card->has_tag(tag_type::drawing)
                    ? "LOG_DRAWN_WITH_CHARACTER"
                    : "LOG_RESPONDED_WITH_CHARACTER"
                : "LOG_PLAYED_CHARACTER", origin_card, origin);
            break;
        case pocket_type::shop_selection:
            origin->m_game->add_log("LOG_BOUGHT_CARD", origin_card, origin);
            break;
        }
    }

    static void log_equipped_card(card *origin_card, player *origin, player *target) {
        if (origin_card->pocket == pocket_type::shop_selection) {
            if (origin == target) {
                origin->m_game->add_log("LOG_BOUGHT_EQUIP", origin_card, origin);
            } else {
                origin->m_game->add_log("LOG_BOUGHT_EQUIP_TO", origin_card, origin, target);
            }
        } else {
            if (origin == target) {
                origin->m_game->add_log("LOG_EQUIPPED_CARD", origin_card, origin);
            } else {
                origin->m_game->add_log("LOG_EQUIPPED_CARD_TO", origin_card, origin, target);
            }
        }
    }

    void apply_target_list(player *origin, card *origin_card, bool is_response, const target_list &targets, const effect_context &ctx) {
        log_played_card(origin_card, origin, is_response);

        if (!ctx.repeating && !ranges::contains(origin_card->get_effect_list(is_response), effect_type::play_card_action, &effect_holder::type)) {
            origin->play_card_action(origin_card);
        }

        std::vector<std::pair<const effect_holder &, const play_card_target &>> delay_effects;
        card_cube_count selected_cubes;

        target_list mth_targets;
        for (const auto &[target, effect] : zip_card_targets(targets, origin_card, is_response)) {
            if (effect.type == effect_type::mth_add) {
                mth_targets.push_back(target);
            } else if (effect.type == effect_type::pay_cube) {
                if (auto *cs = target.get_if<target_type::select_cubes>()) {
                    for (card *c : *cs) {
                        ++selected_cubes[c];
                    }
                } else if (target.is(target_type::self_cubes)) {
                    selected_cubes[origin_card] += effect.target_value;
                }
            } else {
                delay_effects.emplace_back(effect, target);
            }
        }
        for (const auto &[c, ncubes] : selected_cubes) {
            origin->pay_cubes(c, ncubes);
        }
        for (const auto &[effect, target] : delay_effects) {
            enums::visit_indexed([&]<target_type E>(enums::enum_tag_t<E>, auto && ... args) {
                play_visitor<E>{origin, origin_card, effect}.play(ctx, FWD(args) ... );
            }, target);
        }

        origin_card->get_mth(is_response).on_play(origin_card, origin, mth_targets, ctx);
    }

    int get_card_cost(card *origin_card, bool is_response, const std::vector<card *> &modifiers, const effect_context &ctx) {
        if (!is_response && !ctx.repeating && origin_card->pocket != pocket_type::player_table) {
            int cost = origin_card->buy_cost() - ctx.discount;
            for (card *mod_card : modifiers) {
                cost += mod_card->buy_cost();
            }
            return cost;
        } else {
            return 0;
        }
    }

    game_string verify_and_play(player *origin, card *origin_card, bool is_response, const target_list &targets, const modifier_list &modifiers) {
        if (!is_response) {
            if (origin->m_game->pending_requests()) {
                return "ERROR_MUST_RESPOND_TO_REQUEST";
            }
            if (origin->m_game->m_playing != origin) {
                return "ERROR_PLAYER_NOT_IN_TURN";
            }
        }

        effect_context ctx;

        if ((origin_card->pocket == pocket_type::player_hand || origin_card->pocket == pocket_type::shop_selection) && !origin_card->is_brown()) {
            if (is_response) {
                return "ERROR_INVALID_RESPONSE_CARD";
            }

            MAYBE_RETURN(verify_equip_target(origin, origin_card, targets));
            MAYBE_RETURN(verify_modifiers(origin, origin_card, is_response, modifiers, ctx));

            int cost = get_card_cost(origin_card, is_response, get_modifier_cards(modifiers), ctx);
            if (origin->m_gold < cost) {
                return "ERROR_NOT_ENOUGH_GOLD";
            }

            player *target = origin;
            if (!origin_card->self_equippable()) {
                target = targets.front().get<target_type::player>();
            }

            origin->prompt_then(check_prompt_equip(origin_card, origin, target), [=]{
                origin->add_played_card(origin_card, get_modifier_cards(modifiers));

                origin->add_gold(-cost);
                origin_card->on_equip(target);
                log_equipped_card(origin_card, origin, target);
                for (const auto &[mod_card, mod_targets] : modifiers) {
                    apply_target_list(origin, mod_card, is_response, mod_targets, ctx);
                }
                target->equip_card(origin_card);

                if (origin_card->is_green()) {
                    origin_card->inactive = true;
                    origin->m_game->add_update<game_update_type::tap_card>(origin_card, true);
                } else if (origin_card->is_black()) {
                    origin->m_game->draw_shop_card();
                }

                origin->m_game->call_event<event_type::on_equip_card>(origin, target, origin_card);
                origin->m_game->call_event<event_type::on_effect_end>(origin, origin_card);
            });
        } else {
            MAYBE_RETURN(verify_card_targets(origin, origin_card, is_response, targets, modifiers, ctx));

            int cost = get_card_cost(origin_card, is_response, get_modifier_cards(modifiers), ctx);
            if (origin->m_gold < cost) {
                return "ERROR_NOT_ENOUGH_GOLD";
            }

            origin->prompt_then(check_prompt(origin, origin_card, is_response, targets, modifiers, ctx), [=]{
                origin->add_played_card(origin_card, get_modifier_cards(modifiers));

                origin->add_gold(-cost);
                for (const auto &[mod_card, mod_targets] : modifiers) {
                    apply_target_list(origin, mod_card, is_response, mod_targets, ctx);
                }
                apply_target_list(origin, origin_card, is_response, targets, ctx);

                origin->m_game->call_event<event_type::on_effect_end>(origin, origin_card);
            });
        }
        return {};
    }
}