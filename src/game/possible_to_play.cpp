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

    target_list player::make_card_target_set(card *origin_card, const effect_holder &holder) {
        target_list ret;

        for (player &target : m_game->m_players) {
            if (!check_player_filter(origin_card, this, holder.player_filter, &target)) {
                if (holder.target == target_type::player) {
                    ret.emplace_back(enums::enum_tag<target_type::player>, &target);
                } else if (holder.target == target_type::conditional_player) {
                    ret.emplace_back(enums::enum_tag<target_type::conditional_player>, &target);
                }
            } else {
                continue;
            }
            if (holder.target == target_type::card) {
                if (!bool(holder.card_filter & target_card_filter::hand)) {
                    for (card *target_card : target.m_table) {
                        if (target_card != origin_card
                            && (target_card->color == card_color_type::black) == bool(holder.card_filter & target_card_filter::black)
                        ) {
                            ret.emplace_back(enums::enum_tag<target_type::card>, target_card);
                        }
                    }
                }
                if (!bool(holder.card_filter & target_card_filter::table)) {
                    for (card *target_card : target.m_hand) {
                        if (target_card != origin_card) {
                            ret.emplace_back(enums::enum_tag<target_type::card>, target_card);
                        }
                    }
                }
            }
        }
        return ret;
    }

    bool player::is_possible_to_play(card *target_card) {
        if (target_card->color == card_color_type::brown) {
            switch (target_card->modifier) {
            case card_modifier_type::none:
                if (target_card->effects.empty()) return false;
                return std::ranges::all_of(target_card->effects, [&](const effect_holder &holder) {
                    return holder.target == target_type::none
                        || !make_card_target_set(target_card, holder).empty();
                });
            case card_modifier_type::bangmod: {
                effect_holder holder;
                holder.target = target_type::player;
                holder.player_filter = target_player_filter::reachable | target_player_filter::notself;
                return std::ranges::any_of(m_hand, [](card *c) {
                    return c->owner->is_bangcard(c);
                }) && !make_card_target_set(target_card, holder).empty();
            }
            default: return true;
            }
        } else {
            if (m_game->check_flags(game_flags::disable_equipping)) return false;
            if (!target_card->self_equippable()) {
                return !make_equip_set(target_card).empty();
            }
            return true;
        }
    }
}