#ifndef __FILTERS_H__
#define __FILTERS_H__

#include "game_update.h"

namespace banggame {

    using namespace enums::flag_operators;

    namespace filter_impl {
        using namespace banggame;

        int get_player_hp(serial::player origin);
        bool check_player_flags(serial::player origin, player_flags flags);
        bool is_player_alive(serial::player origin);
        player_role get_player_role(serial::player origin);
        int get_player_range_mod(serial::player origin);
        int get_player_weapon_range(serial::player origin);
        int get_distance(serial::player origin, serial::player target);
        bool is_bangcard(serial::player origin, serial::card target);
        card_sign get_card_sign(serial::player origin, serial::card target);
        card_color_type get_card_color(serial::card target);
        pocket_type get_card_pocket(serial::card target);
        card_deck_type get_card_deck(serial::card target);
        bool card_has_tag(serial::card target, tag_type type);
        bool is_cube_slot(serial::card target);
    }

    inline std::string check_player_filter(serial::player origin, target_player_filter filter, serial::player target) {
        if (bool(filter & target_player_filter::dead)) {
            if (filter_impl::get_player_hp(target) > 0) return "ERROR_TARGET_NOT_DEAD";
        } else if (!filter_impl::check_player_flags(target, player_flags::targetable) && !filter_impl::is_player_alive(target)) {
            return "ERROR_TARGET_DEAD";
        }

        if (bool(filter & target_player_filter::self) && target != origin)
            return "ERROR_TARGET_NOT_SELF";

        if (bool(filter & target_player_filter::notself) && target == origin)
            return "ERROR_TARGET_SELF";

        if (bool(filter & target_player_filter::notsheriff) && filter_impl::get_player_role(target) == player_role::sheriff)
            return "ERROR_TARGET_SHERIFF";

        if (bool(filter & (target_player_filter::reachable | target_player_filter::range_1 | target_player_filter::range_2))) {
            int range = filter_impl::get_player_range_mod(origin);
            if (bool(filter & target_player_filter::reachable)) {
                range += filter_impl::get_player_weapon_range(origin);
            } else if (bool(filter & target_player_filter::range_1)) {
                ++range;
            } else if (bool(filter & target_player_filter::range_2)) {
                range += 2;
            }
            if (filter_impl::get_distance(origin, target) > range) {
                return "ERROR_TARGET_NOT_IN_RANGE";
            }
        }

        return {};
    }

    inline std::string check_card_filter(serial::card origin_card, serial::player origin, target_card_filter filter, serial::card target) {
        if (!bool(filter & target_card_filter::can_target_self) && target == origin_card)
            return "ERROR_TARGET_PLAYING_CARD";
        
        if (bool(filter & target_card_filter::cube_slot)) {
            if (!filter_impl::is_cube_slot(target))
                return "ERROR_TARGET_NOT_CUBE_SLOT";
        } else if (filter_impl::get_card_deck(target) == card_deck_type::character) {
            return "ERROR_TARGET_NOT_CARD";
        }

        if (bool(filter & target_card_filter::beer) && !filter_impl::card_has_tag(target, tag_type::beer))
            return "ERROR_TARGET_NOT_BEER";

        if (bool(filter & target_card_filter::bang) && !filter_impl::is_bangcard(origin, target))
            return "ERROR_TARGET_NOT_BANG";

        if (bool(filter & target_card_filter::bangcard) && !filter_impl::card_has_tag(target, tag_type::bangcard))
            return "ERROR_TARGET_NOT_BANG";

        if (bool(filter & target_card_filter::missed) && !filter_impl::card_has_tag(target, tag_type::missedcard))
            return "ERROR_TARGET_NOT_MISSED";

        if (bool(filter & target_card_filter::bronco) && !filter_impl::card_has_tag(target, tag_type::bronco))
            return "ERROR_TARGET_NOT_BRONCO";

        if (bool(filter & target_card_filter::blue) && filter_impl::get_card_color(target) != card_color_type::blue)
            return "ERROR_TARGET_NOT_BLUE_CARD";

        if (bool(filter & target_card_filter::clubs) && filter_impl::get_card_sign(origin, target).suit != card_suit::clubs)
            return "ERROR_TARGET_NOT_CLUBS";

        if (bool(filter & target_card_filter::black) != (filter_impl::get_card_color(target) == card_color_type::black))
            return "ERROR_TARGET_BLACK_CARD";

        if (bool(filter & target_card_filter::table) && filter_impl::get_card_pocket(target) != pocket_type::player_table)
            return "ERROR_TARGET_NOT_TABLE_CARD";

        if (bool(filter & target_card_filter::hand) && filter_impl::get_card_pocket(target) != pocket_type::player_hand)
            return "ERROR_TARGET_NOT_HAND_CARD";

        return {};
    }
}

#endif