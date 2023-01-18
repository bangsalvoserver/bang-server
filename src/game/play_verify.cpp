#include "play_verify.h"

#include "play_visitor.h"

#include "cards/base/requests.h"
#include "effect_list_zip.h"

#include "utils/raii_editor.h"
#include "utils/utils.h"

namespace banggame {

    play_card_verify::play_card_verify(player *origin, card *origin_card, bool is_response, target_list targets, std::vector<card *> modifiers)
        : origin(origin)
        , origin_card(origin_card)
        , playing_card(ranges::contains(modifiers, card_modifier_type::leevankliff, &card::modifier_type) ? origin->get_last_played_card() : origin_card)
        , is_response(is_response)
        , targets(std::move(targets))
        , modifiers(std::move(modifiers)) {}
    
    verify_result play_card_verify::verify_modifiers() const {
        verify_result result;
        auto allowed_modifiers = allowed_modifiers_after(card_modifier_type::none);
        for (card *mod_card : modifiers) {
            if (mod_card->modifier_type() == card_modifier_type::none) {
                return "ERROR_INVALID_MODIFIER_CARD";
            } else if (card *disabler = origin->m_game->get_disabler(mod_card)) {
                return {"ERROR_CARD_DISABLED_BY", mod_card, disabler};
            } else if (auto error = origin->m_game->call_event<event_type::verify_play_card>(origin, mod_card, game_string{})) {
                return error;
            } else if (!bool(allowed_modifiers & modifier_bitset(mod_card->modifier_type()))) {
                return "ERROR_INVALID_MODIFIER_CARD";
            } else if (!allowed_card_with_modifier(origin, mod_card, origin_card)) {
                return "ERROR_INVALID_MODIFIER_CARD";
            }
            result.add(mod_card->modifier.verify(mod_card, origin, playing_card));
            for (const auto &effect : mod_card->effects) {
                result.add(effect.verify(mod_card, origin));
            }
            allowed_modifiers &= allowed_modifiers_after(mod_card->modifier_type());
        }
        return result;
    }

    game_string play_card_verify::verify_duplicates() const {
        std::set<card *> selected_cards;
        std::set<player *> selected_players;
        card_cube_count selected_cubes;

        for (card *mod_card : modifiers) {
            if (!selected_cards.emplace(mod_card).second) {
                return {"ERROR_DUPLICATE_CARD", mod_card};
            }
            for (const auto &effect : mod_card->effects) {
                if (effect.target == target_type::self_cubes) {
                    if (selected_cubes[mod_card] += effect.target_value > mod_card->num_cubes) {
                        return  {"ERROR_NOT_ENOUGH_CUBES_ON", mod_card};
                    }
                }
            }
        }

        auto &effects = is_response ? playing_card->responses : playing_card->effects;
        for (const auto &[target, effect] : zip_card_targets(targets, effects, playing_card->optionals)) {
            if (game_string error = enums::visit_indexed(
                [&]<target_type E>(enums::enum_tag_t<E>, auto && ... args) -> game_string {
                    duplicate_set duplicates = play_visitor<E>{*this, effect}.duplicates(FWD(args) ... );
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
                }, target))
            {
                return error;
            }
        }

        return {};
    }

    void play_card_verify::play_modifiers() const {
        for (card *mod_card : modifiers) {
            origin->log_played_card(mod_card, false);
            auto &effects = is_response ? mod_card->responses : mod_card->effects;
            if (!ranges::contains(effects, effect_type::play_card_action, &effect_holder::type)) {
                origin->play_card_action(mod_card);
            }
            for (effect_holder &e : effects) {
                if (e.target == target_type::none) {
                    e.on_play(mod_card, origin);
                } else if (e.target == target_type::self_cubes) {
                    origin->pay_cubes(mod_card, e.target_value);
                } else {
                    throw std::runtime_error("Invalid target_type in modifier card");
                }
            }
        }
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
        if (auto error = origin->m_game->call_event<event_type::verify_play_card>(origin, origin_card, game_string{})) {
            return error;
        }
        player *target = origin;
        if (!origin_card->self_equippable()) {
            if (targets.size() != 1 || !targets.front().is(target_type::player)) {
                return "ERROR_INVALID_EQUIP_TARGET";
            }
            target = targets.front().get<target_type::player>();
            if (auto error = check_player_filter(origin, origin_card->equip_target, target)) {
                return error;
            }
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

    verify_result play_card_verify::verify_card_targets() const {
        verify_result result;

        if (!playing_card) {
            return "ERROR_INVALID_CARD";
        }

        auto &effects = is_response ? playing_card->responses : playing_card->effects;

        if (playing_card->modifier_type() != card_modifier_type::none) {
            return "ERROR_INVALID_MODIFIER_CARD";
        }
        if (effects.empty()) {
            return "ERROR_EFFECT_LIST_EMPTY";
        }
        if (card *disabler = origin->m_game->get_disabler(playing_card)) {
            return {"ERROR_CARD_DISABLED_BY", playing_card, disabler};
        }
        if (playing_card->inactive) {
            return {"ERROR_CARD_INACTIVE", playing_card};
        }
        if (auto error = origin->m_game->call_event<event_type::verify_play_card>(origin, playing_card, game_string{})) {
            return error;
        }

        result.add(verify_modifiers());
        if (result) return result;

        size_t diff = targets.size() - effects.size();
        if (auto repeatable = playing_card->get_tag_value(tag_type::repeatable)) {
            if (diff < 0 || diff % playing_card->optionals.size() != 0
                || (*repeatable > 0 && diff > (playing_card->optionals.size() * *repeatable)))
            {
                return "ERROR_INVALID_TARGETS";
            }
        } else if (diff != 0 && diff != playing_card->optionals.size()) {
            return "ERROR_INVALID_TARGETS";
        }

        target_list mth_targets;
        for (const auto &[target, effect] : zip_card_targets(targets, effects, playing_card->optionals)) {
            if (!target.is(effect.target)) {
                return "ERROR_INVALID_TARGET_TYPE";
            } else if (effect.type == effect_type::mth_add) {
                mth_targets.push_back(target);
            }
            
            result.add(enums::visit_indexed(
                [&]<target_type E>(enums::enum_tag_t<E>, auto && ... args) {
                    return play_visitor<E>{*this, effect}.verify(FWD(args) ... );
                }, target));
            if (result) return result;
        }

        result.add((is_response ? playing_card->mth_response : playing_card->mth_effect).verify(playing_card, origin, mth_targets));
        if (result) return result;

        result.add_error(verify_duplicates());

        return result;
    }

    game_string play_card_verify::check_prompt() const {
        auto &effects = is_response ? playing_card->responses : playing_card->effects;

        target_list mth_targets;
        for (const auto &[target, effect] : zip_card_targets(targets, effects, playing_card->optionals)) {
            if (effect.type == effect_type::mth_add) {
                mth_targets.push_back(target);
            } else if (auto prompt_message = enums::visit_indexed(
                [&]<target_type E>(enums::enum_tag_t<E>, auto && ... args) {
                    return play_visitor<E>{*this, effect}.prompt(FWD(args) ... );
                }, target))
            {
                return prompt_message;
            }
        }

        return (is_response ? playing_card->mth_response : playing_card->mth_effect).on_prompt(playing_card, origin, mth_targets);
    }

    game_string play_card_verify::check_prompt_equip() const {
        player *target = get_equip_target();
        for (const auto &e : origin_card->equips) {
            if (auto prompt_message = e.on_prompt(origin, origin_card, target)) {
                return prompt_message;
            }
        }
        return {};
    }

    void play_card_verify::do_play_card() const {
        origin->log_played_card(playing_card, is_response);
        if (origin_card != playing_card || std::ranges::none_of(
            ranges::views::concat(modifiers, ranges::views::single(playing_card)),
            [&](card *target_card) {
                return ranges::contains(is_response ? target_card->responses : target_card->effects,
                    effect_type::play_card_action, &effect_holder::type);
            }))
        {
            origin->play_card_action(origin_card);
        }

        std::vector<std::pair<const effect_holder &, const play_card_target &>> delay_effects;
        card_cube_count selected_cubes;

        target_list mth_targets;
        for (const auto &[target, effect] : zip_card_targets(targets, is_response ? playing_card->responses : playing_card->effects, playing_card->optionals)) {
            if (effect.type == effect_type::mth_add) {
                mth_targets.push_back(target);
            } else if (effect.type == effect_type::pay_cube) {
                if (auto *cs = target.get_if<target_type::select_cubes>()) {
                    for (card *c : *cs) {
                        ++selected_cubes[c];
                    }
                } else if (target.is(target_type::self_cubes)) {
                    selected_cubes[playing_card] += effect.target_value;
                }
            } else {
                delay_effects.emplace_back(effect, target);
            }
        }
        for (const auto &[c, ncubes] : selected_cubes) {
            origin->pay_cubes(c, ncubes);
        }
        for (const auto &[e, t] : delay_effects) {
            enums::visit_indexed([&]<target_type E>(enums::enum_tag_t<E>, auto && ... args) {
                play_visitor<E>{*this, e}.play(FWD(args) ... );
            }, t);
        }

        (is_response ? playing_card->mth_response : playing_card->mth_effect).on_play(playing_card, origin, mth_targets);
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
            if (origin_card->is_brown() || ranges::contains(modifiers, card_modifier_type::leevankliff, &card::modifier_type)) {
                if (game_string error = verify_card_targets()) {
                    return error;
                }
                origin->prompt_then(check_prompt(), [*this]{
                    origin->add_played_card(origin_card, modifiers);
                    play_modifiers();
                    do_play_card();
                });
            } else {
                if (game_string error = verify_equip_target()) {
                    return error;
                }
                origin->prompt_then(check_prompt_equip(), [*this]{
                    origin->add_played_card(origin_card, modifiers);
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
        case pocket_type::button_row:
            if (game_string error = verify_card_targets()) {
                return error;
            }
            origin->prompt_then(check_prompt(), [*this]{
                origin->add_played_card(origin_card, modifiers);
                play_modifiers();
                do_play_card();
            });
            break;
        case pocket_type::hidden_deck:
            if (!ranges::contains(modifiers, card_modifier_type::shopchoice, &card::modifier_type)) {
                return "ERROR_INVALID_MODIFIER_CARD";
            }
            [[fallthrough]];
        case pocket_type::shop_selection: {
            if (origin_card->is_brown()) {
                verify_result result = verify_card_targets();
                if (result) return result;
                int cost = origin->get_card_cost(origin_card);
                if (origin->m_gold < cost) {
                    return "ERROR_NOT_ENOUGH_GOLD";
                }
                origin->prompt_then(check_prompt(), [*this, cost]{
                    origin->add_played_card(origin_card, modifiers);
                    play_modifiers();
                    origin->add_gold(-cost);
                    do_play_card();
                    origin->m_game->queue_action([m_game = origin->m_game]{
                        while (m_game->m_shop_selection.size() < 3) {
                            m_game->draw_shop_card();
                        }
                    }, -1);
                });
            } else {
                if (is_response) {
                    return "ERROR_INVALID_RESPONSE_CARD";
                } else if (game_string error = verify_equip_target()) {
                    return error;
                }
                verify_result result = verify_modifiers();
                if (result) return result;
                int cost = origin->get_card_cost(origin_card);
                if (origin->m_gold < cost) {
                    return "ERROR_NOT_ENOUGH_GOLD";
                }
                origin->prompt_then(check_prompt_equip(), [*this, cost]{
                    origin->add_played_card(origin_card, modifiers);
                    player *target = get_equip_target();
                    origin_card->on_equip(target);
                    if (origin == target) {
                        origin->m_game->add_log("LOG_BOUGHT_EQUIP", origin_card, origin);
                    } else {
                        origin->m_game->add_log("LOG_BOUGHT_EQUIP_TO", origin_card, origin, target);
                    }
                    play_modifiers();
                    origin->add_gold(-cost);
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