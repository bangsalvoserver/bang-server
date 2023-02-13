#include "play_verify.h"

#include "play_visitor.h"

#include "cards/base/requests.h"
#include "effect_list_zip.h"

#include "utils/raii_editor.h"
#include "utils/utils.h"

namespace banggame {

    play_card_verify::play_card_verify(player *origin, card *origin_card, bool is_response, target_list targets, std::vector<modifier_pair> modifiers)
        : origin(origin)
        , origin_card(origin_card)
        , playing_card(std::ranges::any_of(modifiers,
            [](const modifier_pair &pair) {
                return pair.card->modifier_type() == card_modifier_type::leevankliff;
            }) ? origin->get_last_played_card() : origin_card)
        , is_response(is_response)
        , targets(std::move(targets))
        , modifiers(std::move(modifiers)) {}

    std::vector<card *> play_card_verify::modifier_cards() const {
        return modifiers
            | ranges::views::transform(&modifier_pair::card)
            | ranges::to<std::vector<card *>>;
    }

    static game_string verify_target_list(player *origin, card *origin_card, bool is_response, const target_list &targets, effect_context &ctx) {
        auto &effects = is_response ? origin_card->responses : origin_card->effects;

        if (effects.empty()) {
            return "ERROR_EFFECT_LIST_EMPTY";
        }
        
        size_t diff = targets.size() - effects.size();
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
        for (const auto &[target, effect] : zip_card_targets(targets, effects, origin_card->optionals)) {
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
                    return play_visitor<E>{origin, origin_card, effect}.verify(ctx, FWD(args) ...);
                }, target));
        }

        auto &mth = is_response ? origin_card->mth_response : origin_card->mth_effect;
        return mth.verify(origin_card, origin, mth_targets, ctx);
    }

    static void apply_target_list(player *origin, card *origin_card, bool is_response, const target_list &targets, const effect_context &ctx) {
        std::vector<std::pair<const effect_holder &, const play_card_target &>> delay_effects;
        card_cube_count selected_cubes;

        target_list mth_targets;
        for (const auto &[target, effect] : zip_card_targets(targets, is_response ? origin_card->responses : origin_card->effects, origin_card->optionals)) {
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

        (is_response ? origin_card->mth_response : origin_card->mth_effect).on_play(origin_card, origin, mth_targets, ctx);
    }
    
    game_string play_card_verify::verify_modifiers_add_context(effect_context &ctx) const {
        auto allowed_modifiers = allowed_modifiers_after(card_modifier_type::none);
        for (const auto &[mod_card, targets] : modifiers) {
            if (!mod_card->is_modifier()) {
                return "ERROR_INVALID_MODIFIER_CARD";
            } else if (card *disabler = origin->m_game->get_disabler(mod_card)) {
                return {"ERROR_CARD_DISABLED_BY", mod_card.get(), disabler};
            }
            MAYBE_RETURN(origin->m_game->call_event<event_type::verify_play_card>(origin, mod_card, game_string{}));
            if (!bool(allowed_modifiers & modifier_bitset(mod_card->modifier_type()))) {
                return "ERROR_INVALID_MODIFIER_CARD";
            } else if (!allowed_card_with_modifier(origin, mod_card, origin_card)) {
                return "ERROR_INVALID_MODIFIER_CARD";
            }
            mod_card->modifier.add_context(mod_card, origin, ctx);
            MAYBE_RETURN(mod_card->modifier.verify(mod_card, origin, playing_card, ctx));
            if (!(is_response ? mod_card->responses : mod_card->effects).empty()) {
                MAYBE_RETURN(verify_target_list(origin, mod_card, is_response, targets, ctx));
            }
            allowed_modifiers &= allowed_modifiers_after(mod_card->modifier_type());
        }
        return {};
    }

    game_string play_card_verify::verify_duplicates() const {
        struct {
            std::set<card *> selected_cards;
            std::set<player *> selected_players;
            card_cube_count selected_cubes;

            game_string operator()(player *origin, card *origin_card, const effect_holder &effect, const play_card_target &target) {
                return enums::visit_indexed([&]<target_type E>(enums::enum_tag_t<E>, auto && ... args) -> game_string {
                    duplicate_set duplicates = play_visitor<E>{origin, origin_card, effect}.duplicates(FWD(args) ... );
                    switch (duplicates.index()) {
                    case 1: {
                        auto &players = std::get<player_set>(duplicates);
                        selected_players.merge(players);
                        if (!players.empty()) return {"ERROR_DUPLICATE_PLAYER", *players.begin()};
                        break;
                    }
                    case 2: {
                        auto &cards = std::get<card_set>(duplicates);
                        selected_cards.merge(cards);
                        if (!cards.empty()) return {"ERROR_DUPLICATE_CARD", *cards.begin()};
                        break;
                    }
                    case 3:
                        for (auto &[card, ncubes] : std::get<card_cube_count>(duplicates)) {
                            if (selected_cubes[card] += ncubes > card->num_cubes) {
                                return {"ERROR_NOT_ENOUGH_CUBES_ON", card};
                            }
                        }
                        break;
                    }
                    return {};
                }, target);
            }
        } check;

        for (const auto &[mod_card, mod_targets] : modifiers) {
            if (!check.selected_cards.emplace(mod_card).second) {
                return {"ERROR_DUPLICATE_CARD", mod_card.get()};
            }
            auto &mod_effects = is_response ? mod_card->responses : mod_card->effects;
            for (const auto &[target, effect] : zip_card_targets(mod_targets, mod_effects, mod_card->optionals)) {
                MAYBE_RETURN(check(origin, mod_card, effect, target));
            }
        }

        auto &effects = is_response ? playing_card->responses : playing_card->effects;
        for (const auto &[target, effect] : zip_card_targets(targets, effects, playing_card->optionals)) {
            MAYBE_RETURN(check(origin, playing_card, effect, target));
        }

        return {};
    }

    game_string play_card_verify::verify_equip_target() const {
        if (is_response) {
            return "ERROR_INVALID_RESPONSE_CARD";
        }        
        if (card *disabler = origin->m_game->get_disabler(origin_card)) {
            return {"ERROR_CARD_DISABLED_BY", origin_card, disabler};
        }
        if (origin->m_game->check_flags(game_flags::disable_equipping)) {
            return "ERROR_CANT_EQUIP_CARDS";
        }
        MAYBE_RETURN(origin->m_game->call_event<event_type::verify_play_card>(origin, origin_card, game_string{}));
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

    player *play_card_verify::get_equip_target() const {
        if (origin_card->self_equippable()) {
            return origin;
        } else {
            return targets.front().get<target_type::player>();
        }
    }

    game_string play_card_verify::verify_card_targets_add_context(effect_context &ctx) const {
        if (!playing_card) {
            return "ERROR_INVALID_CARD";
        }

        if (playing_card->is_modifier()) {
            return "ERROR_INVALID_MODIFIER_CARD";
        }
        if (card *disabler = origin->m_game->get_disabler(playing_card)) {
            return {"ERROR_CARD_DISABLED_BY", playing_card, disabler};
        }
        if (playing_card->inactive) {
            return {"ERROR_CARD_INACTIVE", playing_card};
        }
        MAYBE_RETURN(origin->m_game->call_event<event_type::verify_play_card>(origin, playing_card, game_string{}));
        MAYBE_RETURN(verify_modifiers_add_context(ctx));
        MAYBE_RETURN(verify_target_list(origin, playing_card, is_response, targets, ctx));
        MAYBE_RETURN(verify_duplicates());

        return {};
    }

    game_string play_card_verify::check_prompt(const effect_context &ctx) const {
        auto check = [&](card *c, const target_list &ts) {
            auto &effects = is_response ? c->responses : c->effects;

            target_list mth_targets;
            for (const auto &[target, effect] : zip_card_targets(ts, effects, c->optionals)) {
                if (effect.type == effect_type::mth_add) {
                    mth_targets.push_back(target);
                }
                MAYBE_RETURN(enums::visit_indexed(
                    [&]<target_type E>(enums::enum_tag_t<E>, auto && ... args) {
                        return play_visitor<E>{origin, c, effect}.prompt(ctx, FWD(args) ... );
                    }, target));
            }

            return (is_response ? c->mth_response : c->mth_effect).on_prompt(c, origin, mth_targets, ctx);
        };

        for (const auto &[mod_card, mod_targets] : modifiers) {
            MAYBE_RETURN(mod_card->modifier.on_prompt(mod_card, origin, playing_card));
            MAYBE_RETURN(check(mod_card, mod_targets));
        }

        return check(playing_card, targets);
    }

    game_string play_card_verify::check_prompt_equip() const {
        player *target = get_equip_target();
        for (const auto &e : origin_card->equips) {
            if (auto prompt_message = e.on_prompt(origin_card, origin, target)) {
                return prompt_message;
            }
        }
        return {};
    }

    void play_card_verify::play_modifiers(const effect_context &ctx) const {
        for (const auto &[mod_card, mod_targets] : modifiers) {
            origin->log_played_card(mod_card, is_response);
            auto &effects = is_response ? mod_card->responses : mod_card->effects;
            if (!ranges::contains(effects, effect_type::play_card_action, &effect_holder::type)) {
                origin->play_card_action(mod_card);
            }
            apply_target_list(origin, mod_card, is_response, mod_targets, ctx);
        }
    }

    void play_card_verify::do_play_card(const effect_context &ctx) const {
        origin->log_played_card(playing_card, is_response);
        if (origin_card != playing_card || std::ranges::none_of(
            ranges::views::concat(modifiers | ranges::views::transform(&modifier_pair::card), ranges::views::single(not_null(playing_card))),
            [&](card *target_card) {
                return ranges::contains(is_response ? target_card->responses : target_card->effects,
                    effect_type::play_card_action, &effect_holder::type);
            }))
        {
            origin->play_card_action(origin_card);
        }

        apply_target_list(origin, playing_card, is_response, targets, ctx);
        origin->m_game->call_event<event_type::on_effect_end>(origin, playing_card);
    }

    game_string play_card_verify::verify_and_play() {
        if (!is_response) {
            if (origin->m_game->pending_requests()) {
                return "ERROR_MUST_RESPOND_TO_REQUEST";
            } else if (origin->m_game->m_playing != origin) {
                return "ERROR_PLAYER_NOT_IN_TURN";
            }
        }

        switch(origin_card->pocket) {
        case pocket_type::player_hand:
            if (origin_card->is_brown() || origin_card != playing_card) {
                effect_context ctx;
                MAYBE_RETURN(verify_card_targets_add_context(ctx));
                origin->prompt_then(check_prompt(ctx), [*this, ctx]{
                    origin->add_played_card(origin_card, modifier_cards());
                    play_modifiers(ctx);
                    do_play_card(ctx);
                });
            } else {
                MAYBE_RETURN(verify_equip_target());
                origin->prompt_then(check_prompt_equip(), [*this]{
                    origin->add_played_card(origin_card, modifier_cards());
                    player *target = get_equip_target();
                    origin_card->on_equip(target);
                    if (origin == target) {
                        origin->m_game->add_log("LOG_EQUIPPED_CARD", origin_card, origin);
                    } else {
                        origin->m_game->add_log("LOG_EQUIPPED_CARD_TO", origin_card, origin, target);
                    }
                    target->equip_card(origin_card);
                    if (origin_card->is_green()) {
                        origin_card->inactive = true;
                        origin->m_game->add_update<game_update_type::tap_card>(origin_card, true);
                    }
                    origin->m_game->call_event<event_type::on_equip_card>(origin, target, origin_card);
                    origin->m_game->call_event<event_type::on_effect_end>(origin, origin_card);
                });
            }
            break;
        case pocket_type::scenario_card:
        case pocket_type::wws_scenario_card:
            if (origin_card->pocket == pocket_type::scenario_card && origin_card != origin->m_game->m_scenario_cards.back()) {
                return "ERROR_INVALID_SCENARIO_CARD";
            } else if (origin_card->pocket == pocket_type::wws_scenario_card && origin_card != origin->m_game->m_wws_scenario_cards.back()) {
                return "ERROR_INVALID_SCENARIO_CARD";
            }
            [[fallthrough]];
        case pocket_type::player_character:
        case pocket_type::player_table:
        case pocket_type::button_row: {
            effect_context ctx;
            MAYBE_RETURN(verify_card_targets_add_context(ctx));
            origin->prompt_then(check_prompt(ctx), [*this, ctx]{
                origin->add_played_card(origin_card, modifier_cards());
                play_modifiers(ctx);
                do_play_card(ctx);
            });
            break;
        }
        case pocket_type::hidden_deck:
            if (std::ranges::none_of(modifiers, [](const modifier_pair &pair) { return pair.card->modifier_type() == card_modifier_type::shopchoice; })) {
                return "ERROR_INVALID_MODIFIER_CARD";
            }
            [[fallthrough]];
        case pocket_type::shop_selection: {
            int cost = ranges::accumulate(modifiers | ranges::views::transform([](const modifier_pair &pair) { return pair.card->buy_cost(); }), origin_card->buy_cost());
            if (origin_card->is_brown()) {
                effect_context ctx;
                MAYBE_RETURN(verify_card_targets_add_context(ctx));
                if (is_response) {
                    cost = 0;
                } else if (ctx.discount) {
                    --cost;
                }
                if (origin->m_gold < cost) {
                    return "ERROR_NOT_ENOUGH_GOLD";
                }
                origin->prompt_then(check_prompt(ctx), [*this, cost, ctx]{
                    origin->add_played_card(origin_card, modifier_cards());
                    origin->add_gold(-cost);
                    play_modifiers(ctx);
                    do_play_card(ctx);
                    origin->m_game->queue_action([m_game = origin->m_game]{
                        while (m_game->m_shop_selection.size() < 3) {
                            m_game->draw_shop_card();
                        }
                    }, -1);
                });
            } else {
                effect_context ctx;
                MAYBE_RETURN(verify_equip_target());
                MAYBE_RETURN(verify_modifiers_add_context(ctx));
                if (ctx.discount) --cost;
                if (origin->m_gold < cost) {
                    return "ERROR_NOT_ENOUGH_GOLD";
                }
                origin->prompt_then(check_prompt_equip(), [*this, cost, ctx]{
                    origin->add_played_card(origin_card, modifier_cards());
                    player *target = get_equip_target();
                    origin_card->on_equip(target);
                    if (origin == target) {
                        origin->m_game->add_log("LOG_BOUGHT_EQUIP", origin_card, origin);
                    } else {
                        origin->m_game->add_log("LOG_BOUGHT_EQUIP_TO", origin_card, origin, target);
                    }
                    origin->add_gold(-cost);
                    play_modifiers(ctx);
                    target->equip_card(origin_card);
                    origin->m_game->queue_action([m_game = origin->m_game]{
                        while (m_game->m_shop_selection.size() < 3) {
                            m_game->draw_shop_card();
                        }
                    }, -1);
                });
            }
            break;
        }
        default:
            throw std::runtime_error("play_card: invalid card");
        }
        return {};
    }
}