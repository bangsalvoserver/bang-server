#include "player.h"

#include "game.h"

#include "play_verify.h"

namespace banggame {
    using namespace enums::flag_operators;

    std::vector<player *> player::make_equip_set(card *card) {
        std::vector<player *> ret;
        for (player &p : m_game->m_players) {
            if (!check_player_filter(card, this, card->equip_target, &p) && !p.find_equipped_card(card)) {
                ret.push_back(&p);
            }
        }
        return ret;
    }

    std::vector<player *> player::make_player_target_set(card *origin_card, const effect_holder &holder) {
        std::vector<player *> ret;
        for (player &target : m_game->m_players) {
            if (!check_player_filter(origin_card, this, holder.player_filter, &target)) {
                ret.push_back(&target);
            }
        }
        return ret;
    }

    std::vector<card *> player::make_card_target_set(card *origin_card, const effect_holder &holder) {
        std::vector<card *> ret;
        auto add_if_valid = [&](card *target_card) {
            if (target_card != origin_card || origin_card->has_tag(tag_type::can_target_self)) {
                if (!check_card_filter(origin_card, this, holder.card_filter, target_card)) {
                    ret.push_back(target_card);
                }
            }
        };
        for (player *target : make_player_target_set(origin_card, holder)) {
            std::ranges::for_each(target->m_hand, add_if_valid);
            std::ranges::for_each(target->m_table, add_if_valid);
        }
        return ret;
    }

    bool player::is_possible_to_play(card *target_card, bool is_response) {
        auto check_playable = [&] {
            auto &effects = is_response ? target_card->responses : target_card->effects;
            if (effects.empty()) return false;
            switch (target_card->modifier) {
            case card_modifier_type::none:
                return std::ranges::all_of(effects, [&](const effect_holder &holder) {
                    switch (holder.target) {
                    case target_type::player:
                        return !make_player_target_set(target_card, holder).empty();
                    case target_type::card:
                        return !make_card_target_set(target_card, holder).empty();
                    default:
                        return true;
                    }
                });
            case card_modifier_type::bangmod:
                return std::ranges::any_of(m_hand, [](card *c) {
                    return c->owner->is_bangcard(c);
                }) && !make_player_target_set(target_card, effect_holder{
                    .target {target_type::player},
                    .player_filter {target_player_filter::reachable | target_player_filter::notself}
                }).empty();
            default: return true;
            }
        };
        auto check_equippable = [&] {
            if (m_game->check_flags(game_flags::disable_equipping)) return false;
            if (!target_card->self_equippable()) {
                return !make_equip_set(target_card).empty();
            }
            return true;
        };
        if (!target_card->sign) {
            if (target_card->color == card_color_type::black) {
                return check_equippable();
            } else {
                return check_playable();
            }
        } else if (target_card->pocket == pocket_type::player_table) {
            return check_playable();
        } else if (target_card->color == card_color_type::brown) {
            return check_playable();
        } else {
            return check_equippable();
        }
    }
}