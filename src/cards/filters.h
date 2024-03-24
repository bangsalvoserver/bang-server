#ifndef __FILTERS_H__
#define __FILTERS_H__

#include "card_data.h"
#include "filter_enums.h"
#include "game_enums.h"

#include "game/game_string.h"

namespace banggame::filters {

    namespace detail {
        using namespace banggame;

        template<typename T> struct const_ptr;
        template<typename T> struct const_ptr<T *> { using type = const T *; };
        template<typename T> using const_ptr_t = typename const_ptr<T>::type;

        using player_ptr = unwrap_not_null_t<serial::player>;
        using card_ptr = unwrap_not_null_t<serial::card>;

        using const_player_ptr = const_ptr_t<player_ptr>;
        using const_card_ptr = const_ptr_t<card_ptr>;

        bool check_player_flags(const_player_ptr origin, player_flags flags);
        bool check_game_flags(const_player_ptr origin, game_flags flags);
        player_role get_player_role(player_ptr origin);
        int get_player_range_mod(const_player_ptr origin);
        int get_player_weapon_range(const_player_ptr origin);
        int count_player_hand_cards(player_ptr origin);
        int count_player_table_cards(player_ptr origin);
        int count_player_cubes(player_ptr origin);
        int get_distance(player_ptr origin, player_ptr target);
        card_sign get_card_sign(card_ptr target);
        card_color_type get_card_color(card_ptr target);
        pocket_type get_card_pocket(card_ptr target);
        card_deck_type get_card_deck(card_ptr target);
        std::optional<short> get_card_tag(card_ptr target, tag_type tag);
        bool is_cube_slot(card_ptr target);
        card_ptr get_request_origin_card(player_ptr origin);
        player_ptr get_request_origin(player_ptr origin);
        target_list get_request_target_set(player_ptr origin);
    }

    inline bool is_player_ghost(detail::const_player_ptr origin) {
        return detail::check_player_flags(origin, player_flags::ghost_1)
            || detail::check_player_flags(origin, player_flags::ghost_2)
            || detail::check_player_flags(origin, player_flags::temp_ghost);
    }

    inline bool is_player_alive(detail::const_player_ptr origin) {
        return !detail::check_player_flags(origin, player_flags::dead)
            || is_player_ghost(origin);
    }

    inline const serial::card_list &get_selected_cubes(serial::card origin_card, const effect_context &ctx) {
        static const serial::card_list empty_list;
        auto it = rn::find_if(ctx.selected_cubes, [&](const card_cubes_pair &pair) { return pair.card == origin_card; });
        if (it != ctx.selected_cubes.end()) {
            return it->cubes;
        }
        return empty_list;
    }

    inline game_string check_player_filter(detail::player_ptr origin, target_player_filter filter, detail::player_ptr target, const effect_context &ctx = {}) {
        if (bool(filter & target_player_filter::dead)) {
            if (!bool(filter & target_player_filter::alive) && !detail::check_player_flags(target, player_flags::dead)) {
                return "ERROR_TARGET_NOT_DEAD";
            }
        } else if (!is_player_alive(target)) {
            return "ERROR_TARGET_DEAD";
        }

        if (bool(filter & target_player_filter::self) && target != origin)
            return "ERROR_TARGET_NOT_SELF";

        if (bool(filter & target_player_filter::notself) && target == origin)
            return "ERROR_TARGET_SELF";
        
        if (bool(filter & target_player_filter::notorigin)) {
            auto req_origin = detail::get_request_origin(origin);
            if (!req_origin || req_origin == target) {
                return "ERROR_TARGET_ORIGIN";
            }
        }

        if (bool(filter & target_player_filter::notsheriff) && detail::get_player_role(target) == player_role::sheriff)
            return "ERROR_TARGET_SHERIFF";

        if (bool(filter & target_player_filter::not_empty_hand) && detail::count_player_hand_cards(target) == 0)
            return "ERROR_TARGET_EMPTY_HAND";
        
        if (bool(filter & target_player_filter::not_empty_table) && detail::count_player_table_cards(target) == 0)
            return "ERROR_TARGET_EMPTY_TABLE";

        if (bool(filter & target_player_filter::not_empty_cubes) && detail::count_player_cubes(target) == 0)
            return "ERROR_TARGET_EMPTY_CUBES";
        
        if (bool(filter & target_player_filter::target_set)) {
            target_list target_set = detail::get_request_target_set(target);
            if (rn::none_of(target_set, [&](const play_card_target &t) {
                if (auto *p = t.get_if<target_type::player>()) {
                    return *p == target;
                }
                return false;
            })) {
                return "ERROR_TARGET_NOT_IN_TARGET_SET";
            }
        }

        if (!ctx.ignore_distances && bool(filter & (target_player_filter::reachable | target_player_filter::range_1 | target_player_filter::range_2))) {
            int range = detail::get_player_range_mod(origin);
            if (bool(filter & target_player_filter::reachable)) {
                int weapon_range = detail::get_player_weapon_range(origin);
                if (weapon_range == 0) {
                    return "ERROR_TARGET_NOT_IN_RANGE";
                }
                range += weapon_range;
            } else if (bool(filter & target_player_filter::range_1)) {
                ++range;
            } else if (bool(filter & target_player_filter::range_2)) {
                range += 2;
            }
            if (detail::get_distance(origin, target) > range) {
                return "ERROR_TARGET_NOT_IN_RANGE";
            }
        }

        return {};
    }

    inline bool is_equip_card(detail::card_ptr target) {
        switch (detail::get_card_pocket(target)) {
        case pocket_type::player_hand:
        case pocket_type::shop_selection:
            return detail::get_card_color(target) != card_color_type::brown;
        case pocket_type::train:
            return detail::get_card_deck(target) != card_deck_type::locomotive;
        default:
            return false;
        }
    }

    inline bool is_bang_card(detail::player_ptr origin, detail::card_ptr target) {
        return detail::check_game_flags(origin, game_flags::treat_any_as_bang)
            || detail::check_player_flags(origin, player_flags::treat_any_as_bang)
            || detail::get_card_tag(target, tag_type::bangcard).has_value()
            || detail::check_player_flags(origin, player_flags::treat_missed_as_bang)
            && detail::get_card_tag(target, tag_type::missed).has_value();
    }

    inline int get_card_cost(detail::card_ptr target, bool is_response, const effect_context &ctx) {
        if (!is_response && !ctx.repeat_card && detail::get_card_pocket(target) != pocket_type::player_table) {
            if (ctx.card_choice) {
                target = ctx.card_choice;
            }
            return detail::get_card_tag(target, tag_type::buy_cost).value_or(0) - ctx.discount;
        } else {
            return 0;
        }
    }

    inline game_string check_card_filter(detail::card_ptr origin_card, detail::player_ptr origin, target_card_filter filter, detail::card_ptr target, const effect_context &ctx = {}) {
        if (!bool(filter & target_card_filter::can_target_self) && target == origin_card)
            return "ERROR_TARGET_PLAYING_CARD";
        
        if (bool(filter & target_card_filter::cube_slot)) {
            if (!detail::is_cube_slot(target))
                return "ERROR_TARGET_NOT_CUBE_SLOT";
        } else if (detail::get_card_deck(target) == card_deck_type::character) {
            return "ERROR_TARGET_NOT_CARD";
        }

        if (bool(filter & target_card_filter::beer) && !detail::get_card_tag(target, tag_type::beer))
            return "ERROR_TARGET_NOT_BEER";

        if (bool(filter & target_card_filter::bang) && !is_bang_card(origin, target))
            return "ERROR_TARGET_NOT_BANG";

        if (bool(filter & target_card_filter::bangcard) && !detail::get_card_tag(target, tag_type::bangcard))
            return "ERROR_TARGET_NOT_BANG";

        if (bool(filter & target_card_filter::not_bangcard) && detail::get_card_tag(target, tag_type::bangcard))
            return "ERROR_TARGET_BANG";

        if (bool(filter & target_card_filter::missed) && !detail::get_card_tag(target, tag_type::missed))
            return "ERROR_TARGET_NOT_MISSED";

        if (bool(filter & target_card_filter::missedcard) && !detail::get_card_tag(target, tag_type::missedcard))
            return "ERROR_TARGET_NOT_MISSED";

        if (bool(filter & target_card_filter::not_missedcard) && detail::get_card_tag(target, tag_type::missedcard))
            return "ERROR_TARGET_MISSED";

        if (bool(filter & target_card_filter::bronco) && !detail::get_card_tag(target, tag_type::bronco))
            return "ERROR_TARGET_NOT_BRONCO";

        if (bool(filter & target_card_filter::catbalou_panic)
            && !detail::get_card_tag(target, tag_type::cat_balou)
            && !detail::get_card_tag(target, tag_type::panic))
            return "ERROR_TARGET_NOT_CATBALOU_PANIC";

        card_color_type color = detail::get_card_color(target);

        if (bool(filter & target_card_filter::blue) && color != card_color_type::blue)
            return "ERROR_TARGET_NOT_BLUE_CARD";

        if (bool(filter & target_card_filter::train) && color != card_color_type::train)
            return "ERROR_TARGET_NOT_TRAIN";

        if (bool(filter & target_card_filter::nottrain) && color == card_color_type::train)
            return "ERROR_TARGET_TRAIN";

        if (bool(filter & target_card_filter::blue_or_train) && color != card_color_type::blue && color != card_color_type::train)
            return "ERROR_TARGET_NOT_BLUE_OR_TRAIN";

        if (bool(filter & target_card_filter::black) != (color == card_color_type::black))
            return "ERROR_TARGET_BLACK_CARD";

        card_sign sign = detail::get_card_sign(target);

        if (bool(filter & target_card_filter::hearts) && !sign.is_hearts())
            return "ERROR_TARGET_NOT_HEARTS";

        if (bool(filter & target_card_filter::diamonds) && !sign.is_diamonds())
            return "ERROR_TARGET_NOT_DIAMONDS";

        if (bool(filter & target_card_filter::clubs) && !sign.is_clubs())
            return "ERROR_TARGET_NOT_CLUBS";
        
        if (bool(filter & target_card_filter::spades) && !sign.is_spades())
            return "ERROR_TARGET_NOT_SPADES";
        
        if (bool(filter & target_card_filter::origin_card_suit)) {
            detail::card_ptr req_origin_card = detail::get_request_origin_card(origin);
            if (!req_origin_card) return "ERROR_NO_ORIGIN_CARD_SUIT";
            switch (detail::get_card_sign(req_origin_card).suit) {
                case card_suit::hearts: if (!sign.is_hearts()) { return "ERROR_TARGET_NOT_HEARTS"; } break;
                case card_suit::diamonds: if (!sign.is_diamonds()) { return "ERROR_TARGET_NOT_DIAMONDS"; } break;
                case card_suit::clubs: if (!sign.is_clubs()) { return "ERROR_TARGET_NOT_CLUBS"; } break;
                case card_suit::spades: if (!sign.is_spades()) { return "ERROR_TARGET_NOT_SPADES"; } break;
                default: return "ERROR_NO_ORIGIN_CARD_SUIT";
            }
        }
        
        if (bool(filter & target_card_filter::two_to_nine) && !sign.is_two_to_nine())
            return "ERROR_TARGET_NOT_TWO_TO_NINE";
        
        if (bool(filter & target_card_filter::ten_to_ace) && !sign.is_ten_to_ace())
            return "ERROR_TARGET_NOT_TEN_TO_ACE";

        if (bool(filter & target_card_filter::selection) && detail::get_card_pocket(target) != pocket_type::selection)
            return "ERROR_TARGET_NOT_SELECTION";

        if (bool(filter & target_card_filter::table) && detail::get_card_pocket(target) != pocket_type::player_table)
            return "ERROR_TARGET_NOT_TABLE_CARD";

        if (bool(filter & target_card_filter::hand) && detail::get_card_pocket(target) != pocket_type::player_hand)
            return "ERROR_TARGET_NOT_HAND_CARD";

        return {};
    }
}

#endif