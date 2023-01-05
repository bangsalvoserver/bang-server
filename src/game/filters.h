#ifndef __FILTERS_H__
#define __FILTERS_H__

#include "game_update.h"

namespace banggame {

    namespace filter_impl {
        using namespace banggame;

        using player_ptr = unwrap_not_null_t<serial::player>;
        using card_ptr = unwrap_not_null_t<serial::card>;

        int get_player_hp(player_ptr origin);
        bool check_player_flags(player_ptr origin, player_flags flags);
        bool is_player_alive(player_ptr origin);
        player_role get_player_role(player_ptr origin);
        int get_player_range_mod(player_ptr origin);
        int get_player_weapon_range(player_ptr origin);
        int get_distance(player_ptr origin, player_ptr target);
        bool is_bangcard(player_ptr origin, card_ptr target);
        card_sign get_card_sign(player_ptr origin, card_ptr target);
        card_color_type get_card_color(card_ptr target);
        pocket_type get_card_pocket(card_ptr target);
        card_deck_type get_card_deck(card_ptr target);
        bool card_has_tag(card_ptr target, tag_type type);
        bool is_cube_slot(card_ptr target);
    }

    inline const char *check_player_filter(filter_impl::player_ptr origin, target_player_filter filter, filter_impl::player_ptr target) {
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

        return nullptr;
    }

    inline const char *check_card_filter(filter_impl::card_ptr origin_card, filter_impl::player_ptr origin, target_card_filter filter, filter_impl::card_ptr target) {
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

        return nullptr;
    }

    using modifier_bitset_t = enums::sized_int_t<1 << (enums::num_members_v<card_modifier_type> - 1)>;

    constexpr modifier_bitset_t modifier_bitset(std::same_as<card_modifier_type> auto ... values) {
        return ((1 << enums::to_underlying(values)) | ... | 0);
    }

    inline modifier_bitset_t allowed_modifiers_after(card_modifier_type modifier) {
        switch (modifier) {
        case card_modifier_type::bangmod:
        case card_modifier_type::bandolier:
            return modifier_bitset(card_modifier_type::bangmod, card_modifier_type::bandolier);
        case card_modifier_type::discount:
            return modifier_bitset(card_modifier_type::shopchoice);
        case card_modifier_type::shopchoice:
        case card_modifier_type::leevankliff:
            return modifier_bitset();
        default:
            return ~(-1 << enums::num_members_v<card_modifier_type>);
        }
    }

    inline bool allowed_card_with_modifier(card_modifier_type modifier, filter_impl::player_ptr origin, filter_impl::card_ptr target) {
        switch (modifier) {
        case card_modifier_type::bangmod:
        case card_modifier_type::bandolier:
            if (filter_impl::get_card_pocket(target) == pocket_type::player_hand) {
                return filter_impl::is_bangcard(origin, target);
            } else {
                return filter_impl::card_has_tag(target, tag_type::play_as_bang);
            }
        case card_modifier_type::leevankliff:
            return filter_impl::is_bangcard(origin, target);
        case card_modifier_type::discount:
            return filter_impl::get_card_deck(target) == card_deck_type::goldrush;
        case card_modifier_type::shopchoice:
            return filter_impl::get_card_deck(target) == card_deck_type::goldrush
                && filter_impl::get_card_pocket(target) == pocket_type::hidden_deck;
        case card_modifier_type::belltower:
            switch (filter_impl::get_card_pocket(target)) {
            case pocket_type::player_hand:
            case pocket_type::shop_selection:
                return filter_impl::get_card_color(target) == card_color_type::brown;
            case pocket_type::button_row:
                return false;
            default:
                return true;
            }
        default:
            return true;
        }
    }
}

#endif