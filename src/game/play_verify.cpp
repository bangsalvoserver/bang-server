#include "play_verify.h"

#include "play_visitor.h"

#include "effects/base/requests.h"

#include "utils/raii_editor.h"

namespace banggame {
    using namespace enums::flag_operators;

    opt_error check_player_filter(card *origin_card, player *origin, target_player_filter filter, player *target) {
        if (bool(filter & target_player_filter::dead)) {
            if (target->m_hp > 0) return game_error("ERROR_TARGET_NOT_DEAD");
        } else if (!target->check_player_flags(player_flags::targetable) && !target->alive()) {
            return game_error("ERROR_TARGET_DEAD");
        }

        if (bool(filter & target_player_filter::self) && target != origin)
            return game_error("ERROR_TARGET_NOT_SELF");

        if (bool(filter & target_player_filter::notself) && target == origin)
            return game_error("ERROR_TARGET_SELF");

        if (bool(filter & target_player_filter::notsheriff) && target->m_role == player_role::sheriff)
            return game_error("ERROR_TARGET_SHERIFF");

        if (bool(filter & (target_player_filter::reachable | target_player_filter::range_1 | target_player_filter::range_2))) {
            int distance = origin->m_range_mod;
            if (bool(filter & target_player_filter::reachable)) {
                distance += origin->m_weapon_range;
            } else if (bool(filter & target_player_filter::range_1)) {
                ++distance;
            } else if (bool(filter & target_player_filter::range_2)) {
                distance += 2;
            }
            if (origin->m_game->calc_distance(origin, target) > distance) {
                return game_error("ERROR_TARGET_NOT_IN_RANGE");
            }
        }

        return std::nullopt;
    }

    opt_error check_card_filter(card *origin_card, player *origin, target_card_filter filter, card *target) {
        if (!origin_card->has_tag(tag_type::can_target_self) && target == origin_card)
            return game_error("ERROR_TARGET_PLAYING_CARD");
        
        if (bool(filter & target_card_filter::cube_slot)) {
            if (target != target->owner->m_characters.front() && target->color != card_color_type::orange)
                return game_error("ERROR_TARGET_NOT_CUBE_SLOT");
        } else if (target->deck == card_deck_type::character) {
            return game_error("ERROR_TARGET_NOT_CARD");
        }

        if (bool(filter & target_card_filter::beer) && !target->has_tag(tag_type::beer))
            return game_error("ERROR_TARGET_NOT_BEER");

        if (bool(filter & target_card_filter::bang) && !origin->is_bangcard(target))
            return game_error("ERROR_TARGET_NOT_BANG");

        if (bool(filter & target_card_filter::missed) && !target->has_tag(tag_type::missedcard))
            return game_error("ERROR_TARGET_NOT_MISSED");

        if (bool(filter & target_card_filter::bronco) && !target->has_tag(tag_type::bronco))
            return game_error("ERROR_TARGET_NOT_BRONCO");

        if (bool(filter & target_card_filter::blue) && target->color != card_color_type::blue)
            return game_error("ERROR_TARGET_NOT_BLUE_CARD");

        if (bool(filter & target_card_filter::clubs) && origin->get_card_sign(target).suit != card_suit::clubs)
            return game_error("ERROR_TARGET_NOT_CLUBS");

        if (bool(filter & target_card_filter::black) != (target->color == card_color_type::black))
            return game_error("ERROR_TARGET_BLACK_CARD");

        if (bool(filter & target_card_filter::table) && target->pocket != pocket_type::player_table)
            return game_error("ERROR_TARGET_NOT_TABLE_CARD");

        if (bool(filter & target_card_filter::hand) && target->pocket != pocket_type::player_hand)
            return game_error("ERROR_TARGET_NOT_HAND_CARD");

        return std::nullopt;
    }

    opt_error play_card_verify::verify_modifiers() const {
        for (card *mod_card : modifiers) {
            if (card *disabler = origin->m_game->get_disabler(mod_card)) {
                return game_error("ERROR_CARD_DISABLED_BY", mod_card, disabler);
            }
            switch(mod_card->modifier) {
            case card_modifier_type::bangmod:
            case card_modifier_type::bandolier:
                if (!origin->is_bangcard(card_ptr) && !card_ptr->has_tag(tag_type::play_as_bang))
                    return game_error("ERROR_INVALID_ACTION");
                break;
            case card_modifier_type::leevankliff:
                if (card_ptr != origin->m_last_played_card)
                    return game_error("ERROR_INVALID_ACTION");
                break;
            case card_modifier_type::shopchoice:
            case card_modifier_type::discount:
                if (card_ptr->expansion != card_expansion_type::goldrush)
                    return game_error("ERROR_INVALID_ACTION");
                break;
            case card_modifier_type::belltower:
                switch (card_ptr->pocket) {
                case pocket_type::player_hand:
                    if (card_ptr->color != card_color_type::brown)
                        return game_error("ERROR_INVALID_ACTION");
                    break;
                case pocket_type::player_table:
                    if (card_ptr->effects.empty())
                        return game_error("ERROR_INVALID_ACTION");
                    break;
                default:
                    if (card_ptr->color == card_color_type::black)
                        return game_error("ERROR_INVALID_ACTION");
                }
                break;
            default:
                return game_error("ERROR_INVALID_ACTION");
            }
            for (const auto &effect : mod_card->effects) {
                if (opt_error e = effect.verify(mod_card, origin)) {
                    return e;
                }
            }
        }
        return std::nullopt;
    }

    void play_card_verify::play_modifiers() const {
        for (card *mod_card : modifiers) {
            origin->log_played_card(mod_card, false);
            origin->play_card_action(mod_card);
            for (effect_holder &e : mod_card->effects) {
                e.on_play(mod_card, origin, effect_flags{});
            }
        }
    }

    opt_error play_card_verify::verify_equip_target() const {
        if (card *disabler = origin->m_game->get_disabler(card_ptr)) {
            return game_error("ERROR_CARD_DISABLED_BY", card_ptr, disabler);
        }
        if (origin->m_game->check_flags(game_flags::disable_equipping)) {
            return game_error("ERROR_CANT_EQUIP_CARDS");
        }
        player *target = origin;
        if (!card_ptr->self_equippable()) {
            if (targets.size() != 1 || !targets.front().is(target_type::player)) {
                return game_error("ERROR_INVALID_ACTION");
            }
            target = targets.front().get<target_type::player>();
            if (auto error = check_player_filter(card_ptr, origin, card_ptr->equip_target, target)) {
                return error;
            }
        }
        if (auto *card = target->find_equipped_card(card_ptr)) {
            return game_error("ERROR_DUPLICATED_CARD", card);
        }
        if (card_ptr->color == card_color_type::orange && origin->m_game->num_cubes < 3) {
            return game_error("ERROR_NOT_ENOUGH_CUBES");
        }
        return std::nullopt;
    }

    player *play_card_verify::get_equip_target() const {
        if (card_ptr->self_equippable()) {
            return origin;
        } else {
            return targets.front().get<target_type::player>();
        }
    }

    opt_error play_card_verify::verify_card_targets() const {
        auto &effects = is_response ? card_ptr->responses : card_ptr->effects;

        if (card *disabler = origin->m_game->get_disabler(card_ptr)) {
            return game_error("ERROR_CARD_DISABLED_BY", card_ptr, disabler);
        }
        if (card_ptr->inactive) {
            return game_error("ERROR_CARD_INACTIVE", card_ptr);
        }

        if (origin->m_mandatory_card && origin->m_mandatory_card != card_ptr
            && std::ranges::find(origin->m_mandatory_card->effects, effect_type::banglimit, &effect_holder::type) != origin->m_mandatory_card->effects.end()
            && std::ranges::find(effects, effect_type::banglimit, &effect_holder::type) != effects.end())
        {
            return game_error("ERROR_MANDATORY_CARD", origin->m_mandatory_card);
        }

        if (origin->m_forced_card
            && card_ptr != origin->m_forced_card
            && std::ranges::find(modifiers, origin->m_forced_card) == modifiers.end()) {
            return game_error("ERROR_INVALID_ACTION");
        }

        struct {
            raii_editor_stack<int8_t> data;
            void add(int8_t &value, int8_t diff) {
                data.add(value, value + diff);
            }
        } editors;

        for (card *c : modifiers) {
            switch (c->modifier) {
            case card_modifier_type::belltower: editors.add(origin->m_range_mod, 50); break;
            case card_modifier_type::bandolier: editors.add(origin->m_bangs_per_turn, 1); break;
            case card_modifier_type::leevankliff: editors.add(origin->m_bangs_per_turn, 10); break;
            }
        }

        int diff = targets.size() - effects.size();
        if (auto repeatable = card_ptr->get_tag_value(tag_type::repeatable)) {
            if (diff < 0 || diff % card_ptr->optionals.size() != 0
                || (*repeatable > 0 && diff > (card_ptr->optionals.size() * *repeatable)))
            {
                return game_error("ERROR_INVALID_TARGETS");
            }
        } else if (diff != 0 && diff != card_ptr->optionals.size()) {
            return game_error("ERROR_INVALID_TARGETS");
        }
        
        auto effect_it = effects.begin();
        auto effect_end = effects.end();

        target_list mth_targets;
        for (const auto &t : targets) {
            const auto &e = *effect_it;
            if (++effect_it == effect_end) {
                effect_it = card_ptr->optionals.begin();
                effect_end = card_ptr->optionals.end();
            }

            if (!t.is(e.target)) {
                return game_error("ERROR_INVALID_ACTION");
            } else if (e.type == effect_type::mth_add) {
                mth_targets.push_back(t);
            }
            
            if (auto error = enums::visit_indexed(
                [this, &e]<target_type E>(enums::enum_tag_t<E>, auto && ... args) {
                    return play_visitor<E>{}.verify(this, e, std::forward<decltype(args)>(args) ... );
                }, t))
            {
                return error;
            }
        }

        return card_ptr->multi_target_handler.verify(card_ptr, origin, mth_targets);
    }

    opt_fmt_str play_card_verify::check_prompt() const {
        auto &effects = is_response ? card_ptr->responses : card_ptr->effects;
        
        auto effect_it = effects.begin();
        auto effect_end = effects.end();

        target_list mth_targets;
        for (const auto &t : targets) {
            const auto &e = *effect_it;
            if (++effect_it == effect_end) {
                effect_it = card_ptr->optionals.begin();
                effect_end = card_ptr->optionals.end();
            }

            if (e.type == effect_type::mth_add) {
                mth_targets.push_back(t);
            } else if (auto prompt_message = enums::visit_indexed(
                [this, &e]<target_type E>(enums::enum_tag_t<E>, auto && ... args) {
                    return play_visitor<E>{}.prompt(this, e, std::forward<decltype(args)>(args) ... );
                }, t))
            {
                return prompt_message;
            }
        }

        return card_ptr->multi_target_handler.on_prompt(card_ptr, origin, mth_targets);
    }

    opt_fmt_str play_card_verify::check_prompt_equip() const {
        player *target = get_equip_target();
        for (const auto &e : card_ptr->equips) {
            if (auto prompt_message = e.on_prompt(card_ptr, target)) {
                return prompt_message;
            }
        }
        return std::nullopt;
    }

    void play_card_verify::do_play_card() const {
        if (origin->m_mandatory_card == card_ptr) {
            origin->m_mandatory_card = nullptr;
        }
        origin->m_forced_card = nullptr;
        
        auto &effects = is_response ? card_ptr->responses : card_ptr->effects;
        origin->log_played_card(card_ptr, is_response);
        if (std::ranges::find(effects, effect_type::play_card_action, &effect_holder::type) == effects.end()) {
            origin->play_card_action(card_ptr);
        }
        
        auto effect_it = effects.begin();
        auto effect_end = effects.end();

        target_list mth_targets;
        for (const auto &t : targets) {
            auto &e = *effect_it;
            if (++effect_it == effect_end) {
                effect_it = card_ptr->optionals.begin();
                effect_end = card_ptr->optionals.end();
            }

            if (e.type == effect_type::mth_add) {
                mth_targets.push_back(t);
            } else {
                enums::visit_indexed([this, &e]<target_type E>(enums::enum_tag_t<E>, auto && ... args) {
                    play_visitor<E>{}.play(this, e, std::forward<decltype(args)>(args) ... );
                }, t);
            }
        }

        card_ptr->multi_target_handler.on_play(card_ptr, origin, mth_targets);
        origin->m_game->call_event<event_type::on_effect_end>(origin, card_ptr);
    }

    opt_error play_card_verify::verify_and_play() {
        switch(card_ptr->pocket) {
        case pocket_type::player_hand:
            if (!modifiers.empty() && modifiers.front()->modifier == card_modifier_type::leevankliff) {
                card *bang_card = std::exchange(card_ptr, origin->m_last_played_card);
                if (!origin->is_bangcard(bang_card)) {
                    return game_error("ERROR_INVALID_ACTION");
                }
                if (auto error = verify_modifiers()) {
                    return error;
                } else if (auto error = verify_card_targets()) {
                    return error;
                }
                origin->prompt_then(check_prompt(), [*this, bang_card]{
                    origin->m_game->move_card(bang_card, pocket_type::discard_pile);
                    origin->m_game->call_event<event_type::on_play_hand_card>(origin, bang_card);
                    do_play_card();
                    origin->set_last_played_card(nullptr);
                });
            } else if (card_ptr->color == card_color_type::brown) {
                if (auto error = verify_modifiers()) {
                    return error;
                } else if (auto error = verify_card_targets()) {
                    return error;
                }
                origin->prompt_then(check_prompt(), [*this]{
                    play_modifiers();
                    do_play_card();
                    origin->set_last_played_card(card_ptr);
                });
            } else {
                if (auto error = verify_equip_target()) {
                    return error;
                }
                origin->prompt_then(check_prompt_equip(), [*this]{
                    player *target = get_equip_target();
                    if (origin->m_mandatory_card == card_ptr) {
                        origin->m_mandatory_card = nullptr;
                    }
                    origin->m_forced_card = nullptr;
                    card_ptr->on_equip(target);
                    if (origin == target) {
                        origin->m_game->add_log("LOG_EQUIPPED_CARD", card_ptr, origin);
                    } else {
                        origin->m_game->add_log("LOG_EQUIPPED_CARD_TO", card_ptr, origin, target);
                    }
                    target->equip_card(card_ptr);
                    switch (card_ptr->color) {
                    case card_color_type::blue:
                        if (origin->m_game->has_expansion(card_expansion_type::armedanddangerous)) {
                            origin->queue_request_add_cube(card_ptr);
                        }
                        break;
                    case card_color_type::green:
                        card_ptr->inactive = true;
                        origin->m_game->add_update<game_update_type::tap_card>(card_ptr->id, true);
                        break;
                    case card_color_type::orange:
                        origin->add_cubes(card_ptr, 3);
                        break;
                    }
                    origin->m_game->call_event<event_type::on_equip_card>(origin, target, card_ptr);
                    origin->set_last_played_card(nullptr);
                    origin->m_game->call_event<event_type::on_effect_end>(origin, card_ptr);
                });
            }
            break;
        case pocket_type::player_character:
        case pocket_type::player_table:
        case pocket_type::scenario_card:
        case pocket_type::specials:
            if (auto error = verify_modifiers()) {
                return error;
            } else if (auto error = verify_card_targets()) {
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
                return game_error("ERROR_INVALID_ACTION");
            }
            [[fallthrough]];
        case pocket_type::shop_selection: {
            int cost = card_ptr->buy_cost();
            if (auto error = verify_modifiers()) {
                return error;
            }
            for (card *c : modifiers) {
                switch (c->modifier) {
                case card_modifier_type::discount:
                    --cost;
                    break;
                case card_modifier_type::shopchoice:
                    if (c->get_tag_value(tag_type::shopchoice) != card_ptr->get_tag_value(tag_type::shopchoice)) {
                        return game_error("ERROR_INVALID_ACTION");
                    }
                    cost += c->buy_cost();
                    break;
                }
            }
            if (origin->m_game->m_shop_selection.size() > 3) {
                cost = 0;
            }
            if (origin->m_gold < cost) {
                return game_error("ERROR_NOT_ENOUGH_GOLD");
            }
            if (card_ptr->color == card_color_type::brown) {
                if (auto error = verify_card_targets()) {
                    return error;
                }
                origin->prompt_then(check_prompt(), [*this, cost]{
                    play_modifiers();
                    origin->add_gold(-cost);
                    do_play_card();
                    origin->set_last_played_card(nullptr);
                    origin->m_game->queue_action([m_game = origin->m_game]{
                        while (m_game->m_shop_selection.size() < 3) {
                            m_game->draw_shop_card();
                        }
                    });
                });
            } else {
                if (auto error = verify_equip_target()) {
                    return error;
                }
                origin->prompt_then(check_prompt_equip(), [*this, cost]{
                    player *target = get_equip_target();
                    origin->m_forced_card = nullptr;
                    card_ptr->on_equip(target);
                    if (origin == target) {
                        origin->m_game->add_log("LOG_BOUGHT_EQUIP", card_ptr, origin);
                    } else {
                        origin->m_game->add_log("LOG_BOUGHT_EQUIP_TO", card_ptr, origin, target);
                    }
                    play_modifiers();
                    origin->add_gold(-cost);
                    target->equip_card(card_ptr);
                    origin->set_last_played_card(nullptr);
                    origin->m_game->queue_action([m_game = origin->m_game]{
                        while (m_game->m_shop_selection.size() < 3) {
                            m_game->draw_shop_card();
                        }
                    });
                });
            }
            break;
        }
        default:
            throw std::runtime_error("play_card: invalid card");
        }
        return std::nullopt;
    }
}