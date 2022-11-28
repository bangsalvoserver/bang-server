#include "play_verify.h"

#include "play_visitor.h"

#include "cards/base/requests.h"
#include "effect_list_zip.h"

#include "utils/raii_editor.h"
#include "utils/utils.h"

namespace banggame {
    
    game_string play_card_verify::verify_modifiers() const {
        for (card *mod_card : modifiers) {
            if (card *disabler = origin->m_game->get_disabler(mod_card)) {
                return {"ERROR_CARD_DISABLED_BY", mod_card, disabler};
            }
            switch(mod_card->modifier) {
            case card_modifier_type::bangmod:
            case card_modifier_type::bandolier:
                if (origin_card->pocket == pocket_type::player_hand) {
                    if (!origin->is_bangcard(origin_card)) {
                        return "ERROR_INVALID_MODIFIER_CARD";
                    }
                } else if (!origin_card->has_tag(tag_type::play_as_bang)) {
                    return "ERROR_INVALID_MODIFIER_CARD";
                }
                break;
            case card_modifier_type::leevankliff:
                if (origin_card != origin->m_last_played_card)
                    return "ERROR_INVALID_MODIFIER_CARD";
                break;
            case card_modifier_type::shopchoice:
            case card_modifier_type::discount:
                if (origin_card->expansion != card_expansion_type::goldrush)
                    return "ERROR_INVALID_MODIFIER_CARD";
                break;
            case card_modifier_type::belltower:
                switch (origin_card->pocket) {
                case pocket_type::player_hand:
                    if (origin_card->color != card_color_type::brown)
                        return "ERROR_INVALID_MODIFIER_CARD";
                    break;
                case pocket_type::player_table:
                    if (origin_card->effects.empty())
                        return "ERROR_INVALID_MODIFIER_CARD";
                    break;
                default:
                    if (origin_card->color == card_color_type::black)
                        return "ERROR_INVALID_MODIFIER_CARD";
                }
                if (std::ranges::none_of(origin_card->effects, 
                    [](target_player_filter filter) {
                        return bool(filter & (target_player_filter::range_1 | target_player_filter::range_2 | target_player_filter::reachable));
                    }, &effect_holder::player_filter))
                {
                    return "ERROR_INVALID_MODIFIER_CARD";
                }
                break;
            default:
                return "ERROR_INVALID_MODIFIER_CARD";
            }
            for (const auto &effect : mod_card->effects) {
                if (game_string e = effect.verify(mod_card, origin)) {
                    return e;
                }
            }
        }
        return {};
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

        auto &effects = is_response ? origin_card->responses : origin_card->effects;
        for (const auto &[target, effect] : zip_card_targets(targets, effects, origin_card->optionals)) {
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
            origin->play_card_action(mod_card);
            for (effect_holder &e : mod_card->effects) {
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
        if (origin_card->color == card_color_type::orange && origin->m_game->num_cubes < 3) {
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

    struct check_disabler {
        game *m_game = nullptr;
        event_card_key key;

        template<event_type E, invocable_for_event<E> Function>
        check_disabler(game *m_game, event_card_key key, enums::enum_tag_t<E>, Function &&fun)
            : m_game(m_game), key(key)
        {
            m_game->add_listener<E>(key, std::move(fun));
        }

        check_disabler(const check_disabler &) = delete;
        check_disabler(check_disabler &&other) noexcept
            : m_game(std::exchange(other.m_game, nullptr))
            , key(other.key) {}

        check_disabler &operator = (const check_disabler &) = delete;
        check_disabler &operator = (check_disabler &&other) noexcept {
            m_game = std::exchange(other.m_game, nullptr);
            key = other.key;
            return *this;
        }

        ~check_disabler() {
            if (m_game) {
                m_game->remove_listeners(key);
                m_game = nullptr;
            }
        }
    };

    game_string play_card_verify::verify_card_targets() const {
        auto &effects = is_response ? origin_card->responses : origin_card->effects;

        if (effects.empty()) {
            return "ERROR_EFFECT_LIST_EMPTY";
        }
        if (card *disabler = origin->m_game->get_disabler(origin_card)) {
            return {"ERROR_CARD_DISABLED_BY", origin_card, disabler};
        }
        if (origin_card->inactive) {
            return {"ERROR_CARD_INACTIVE", origin_card};
        }
        if (auto error = origin->m_game->call_event<event_type::verify_play_card>(origin, origin_card, game_string{})) {
            return error;
        }

        std::vector<check_disabler> check_disablers;
        for (card *c : modifiers) {
            switch (c->modifier) {
            case card_modifier_type::belltower:
                check_disablers.emplace_back(origin->m_game, event_card_key{origin_card, -1},
                    enums::enum_tag<event_type::apply_distance_modifier>, [origin=origin](player *p, int &value) {
                        if (p == origin) {
                            value = 1;
                        }
                    });
                break;
            case card_modifier_type::bandolier:
            case card_modifier_type::leevankliff:
                check_disablers.emplace_back(origin->m_game, event_card_key{origin_card, -1},
                    enums::enum_tag<event_type::count_bangs_played>, [origin=origin](player *p, int &value) {
                        if (p == origin) {
                            value = 0;
                        }
                    });
                break;
            }
        }

        if (game_string error = verify_modifiers()) {
            return error;
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
            }
            
            if (game_string error = enums::visit_indexed(
                [&]<target_type E>(enums::enum_tag_t<E>, auto && ... args) {
                    return play_visitor<E>{*this, effect}.verify(FWD(args) ... );
                }, target))
            {
                return error;
            }
        }

        if (game_string error = (is_response ? origin_card->mth_response : origin_card->mth_effect).verify(origin_card, origin, mth_targets)) {
            return error;
        }

        if (game_string error = verify_duplicates()) {
            return error;
        }

        return {};
    }

    game_string play_card_verify::check_prompt() const {
        auto &effects = is_response ? origin_card->responses : origin_card->effects;

        target_list mth_targets;
        for (const auto &[target, effect] : zip_card_targets(targets, effects, origin_card->optionals)) {
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

        return (is_response ? origin_card->mth_response : origin_card->mth_effect).on_prompt(origin_card, origin, mth_targets);
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
        auto &effects = is_response ? origin_card->responses : origin_card->effects;
        origin->log_played_card(origin_card, is_response);
        if (std::ranges::find(effects, effect_type::play_card_action, &effect_holder::type) == effects.end()) {
            origin->play_card_action(origin_card);
        }

        std::vector<std::pair<const effect_holder &, const play_card_target &>> delay_effects;
        card_cube_count selected_cubes;

        target_list mth_targets;
        for (const auto &[target, effect] : zip_card_targets(targets, effects, origin_card->optionals)) {
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
        for (const auto &[e, t] : delay_effects) {
            enums::visit_indexed([&]<target_type E>(enums::enum_tag_t<E>, auto && ... args) {
                play_visitor<E>{*this, e}.play(FWD(args) ... );
            }, t);
        }

        (is_response ? origin_card->mth_response : origin_card->mth_effect).on_play(origin_card, origin, mth_targets);
        origin->m_game->call_event<event_type::on_effect_end>(origin, origin_card);
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
            if (!modifiers.empty() && modifiers.front()->modifier == card_modifier_type::leevankliff) {
                card *bang_card = std::exchange(origin_card, origin->m_last_played_card);
                if (!origin->is_bangcard(bang_card)) {
                    return "ERROR_INVALID_MODIFIER_CARD";
                }
                if (game_string error = verify_card_targets()) {
                    return error;
                }
                origin->prompt_then(check_prompt(), [*this, bang_card]{
                    origin->m_game->move_card(bang_card, pocket_type::discard_pile);
                    origin->m_game->call_event<event_type::on_play_hand_card>(origin, bang_card);
                    do_play_card();
                    origin->set_last_played_card(nullptr);
                });
            } else if (origin_card->color == card_color_type::brown) {
                if (game_string error = verify_card_targets()) {
                    return error;
                }
                origin->prompt_then(check_prompt(), [*this]{
                    play_modifiers();
                    do_play_card();
                    origin->set_last_played_card(origin_card);
                });
            } else {
                if (game_string error = verify_equip_target()) {
                    return error;
                }
                origin->prompt_then(check_prompt_equip(), [*this]{
                    player *target = get_equip_target();
                    origin_card->on_equip(target);
                    if (origin == target) {
                        origin->m_game->add_log("LOG_EQUIPPED_CARD", origin_card, origin);
                    } else {
                        origin->m_game->add_log("LOG_EQUIPPED_CARD_TO", origin_card, origin, target);
                    }
                    target->equip_card(origin_card);
                    if (origin_card->color == card_color_type::green) {
                        origin_card->inactive = true;
                        origin->m_game->add_update<game_update_type::tap_card>(origin_card, true);
                    }
                    origin->m_game->call_event<event_type::on_equip_card>(origin, target, origin_card);
                    origin->set_last_played_card(nullptr);
                    origin->m_game->call_event<event_type::on_effect_end>(origin, origin_card);
                });
            }
            break;
        case pocket_type::player_character:
        case pocket_type::player_table:
        case pocket_type::scenario_card:
        case pocket_type::button_row:
            if (game_string error = verify_card_targets()) {
                return error;
            }
            origin->prompt_then(check_prompt(), [*this]{
                play_modifiers();
                do_play_card();
                origin->set_last_played_card(nullptr);
            });
            break;
        case pocket_type::hidden_deck:
            if (std::ranges::find(modifiers, card_modifier_type::shopchoice, &card::modifier) == modifiers.end()) {
                return "ERROR_INVALID_MODIFIER_CARD";
            }
            [[fallthrough]];
        case pocket_type::shop_selection: {
            int cost = origin_card->buy_cost();
            for (card *c : modifiers) {
                switch (c->modifier) {
                case card_modifier_type::discount:
                    --cost;
                    break;
                case card_modifier_type::shopchoice:
                    if (c->get_tag_value(tag_type::shopchoice) != origin_card->get_tag_value(tag_type::shopchoice)) {
                        return "ERROR_INVALID_MODIFIER_CARD";
                    }
                    cost += c->buy_cost();
                    break;
                }
            }
            if (origin->m_game->m_shop_selection.size() > 3) {
                // can only happen when playing Josh McCloud
                cost = 0;
            }
            if (origin->m_gold < cost) {
                return "ERROR_NOT_ENOUGH_GOLD";
            }
            if (origin_card->color == card_color_type::brown) {
                if (game_string error = verify_card_targets()) {
                    return error;
                }
                origin->prompt_then(check_prompt(), [*this, cost]{
                    play_modifiers();
                    origin->add_gold(-cost);
                    do_play_card();
                    origin->set_last_played_card(origin_card);
                    origin->m_game->queue_action([m_game = origin->m_game]{
                        while (m_game->m_shop_selection.size() < 3) {
                            m_game->draw_shop_card();
                        }
                    }, -1);
                });
            } else {
                if (!cost) {
                    is_response = false;
                } else if (is_response) {
                    return "ERROR_INVALID_RESPONSE_CARD";
                } else if (game_string error = verify_modifiers()) {
                    return error;
                } else if (game_string error = verify_equip_target()) {
                    return error;
                }
                origin->prompt_then(check_prompt_equip(), [*this, cost]{
                    if (!cost) {
                        origin->m_game->pop_request();
                    }
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
                    origin->set_last_played_card(nullptr);
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