#include "filters.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"

#include "game_table.h"

namespace banggame {

    game_string check_player_filter(const_card_ptr origin_card, const_player_ptr origin, enums::bitset<target_player_filter> filter, const_player_ptr target, const effect_context &ctx) {
        if (!filter.check(target_player_filter::dead_or_alive)
            && filter.check(target_player_filter::dead) == target->alive()
        ) {
            return {"ERROR_TARGET_DEAD", origin_card, target};
        }

        if (filter.check(target_player_filter::self) && target != origin)
            return {"ERROR_TARGET_NOT_SELF", origin_card};

        if (filter.check(target_player_filter::notself) && target == origin)
            return {"ERROR_TARGET_SELF", origin_card};
        
        if (filter.check(target_player_filter::notorigin)) {
            auto req = origin->m_game->top_request();
            if (!req || req->origin == target) {
                return {"ERROR_TARGET_ORIGIN", origin_card};
            }
        }

        if (filter.check(target_player_filter::notsheriff) && target->m_role == player_role::sheriff)
            return {"ERROR_TARGET_SHERIFF", origin_card, target};
        
        if (filter.check(target_player_filter::legend) && target->get_character()->deck != card_deck_type::legends)
            return {"ERROR_TARGET_NOT_LEGEND", origin_card, target};

        if (filter.check(target_player_filter::not_empty_hand) && target->empty_hand())
            return {"ERROR_TARGET_EMPTY_HAND", origin_card, target};
        
        if (filter.check(target_player_filter::not_empty_table) && target->empty_table())
            return {"ERROR_TARGET_EMPTY_TABLE", origin_card, target};

        if (filter.check(target_player_filter::not_empty_cubes) && target->count_cubes() == 0)
            return {"ERROR_TARGET_EMPTY_CUBES", origin_card, target};
        
        if (filter.check(target_player_filter::target_set)) {
            auto req = origin->m_game->top_request<interface_target_set_players>(target_is{origin});
            if (!req || !req->in_target_set(target)) {
                return {"ERROR_TARGET_NOT_IN_TARGET_SET", origin_card, target};
            }
        }

        if (!ctx.ignore_distances && !origin->check_player_flags(player_flag::ignore_distances)
            && (filter.check(target_player_filter::reachable) || filter.check(target_player_filter::range_1) || filter.check(target_player_filter::range_2))
        ) {
            int range = origin->get_range_mod();
            if (filter.check(target_player_filter::reachable)) {
                int weapon_range = origin->get_weapon_range();
                if (weapon_range == 0) {
                    return {"ERROR_TARGET_NOT_IN_RANGE", origin_card, target};
                }
                range += weapon_range;
            } else if (filter.check(target_player_filter::range_1)) {
                ++range;
            } else if (filter.check(target_player_filter::range_2)) {
                range += 2;
            }
            if (origin->m_game->calc_distance(origin, target) > range) {
                return {"ERROR_TARGET_NOT_IN_RANGE", origin_card, target};
            }
        }

        return {};
    }

    game_string check_card_filter(const_card_ptr origin_card, const_player_ptr origin, enums::bitset<target_card_filter> filter, const_card_ptr target, const effect_context &ctx) {
        if (!filter.check(target_card_filter::can_target_self) && target == origin_card)
            return "ERROR_TARGET_PLAYING_CARD";

        if (filter.check(target_card_filter::target_set)) {
            auto req = origin->m_game->top_request<interface_target_set_cards>(target_is{origin});
            if (!req || !req->in_target_set(target)) {
                return {"ERROR_TARGET_NOT_IN_TARGET_SET", origin_card, target};
            }
        } else if (filter.check(target_card_filter::cube_slot)) {
            if (!target->owner)
                return "ERROR_CARD_HAS_NO_OWNER";
            if (target != target->owner->get_character() && !(target->is_orange() && target->pocket == pocket_type::player_table))
                return "ERROR_TARGET_NOT_CUBE_SLOT";
        } else {
            if (filter.check(target_card_filter::selection) != (target->pocket == pocket_type::selection))
                return "ERROR_TARGET_NOT_SELECTION";
            
            switch (target->pocket) {
            case pocket_type::player_hand:
            case pocket_type::player_table:
            case pocket_type::selection:
                break;
            default:
                return "ERROR_CARD_NOT_TARGETABLE";
            }
        }

        if (filter.check(target_card_filter::beer) && !target->has_tag(tag_type::beer))
            return "ERROR_TARGET_NOT_BEER";

        if (filter.check(target_card_filter::bang) && !target->is_bang_card(origin))
            return "ERROR_TARGET_NOT_BANG";

        if (filter.check(target_card_filter::used_bang) && !(origin->m_game->check_flags(game_flag::showdown) || target->is_bang_card(origin)))
            return "ERROR_TARGET_NOT_BANG";

        if (filter.check(target_card_filter::bangcard) && !target->has_tag(tag_type::bangcard))
            return "ERROR_TARGET_NOT_BANG";

        if (filter.check(target_card_filter::not_bangcard) && target->has_tag(tag_type::bangcard))
            return "ERROR_TARGET_BANG";

        if (filter.check(target_card_filter::missed) && !target->has_tag(tag_type::missed))
            return "ERROR_TARGET_NOT_MISSED";

        if (filter.check(target_card_filter::missedcard) && !target->has_tag(tag_type::missedcard))
            return "ERROR_TARGET_NOT_MISSED";

        if (filter.check(target_card_filter::not_missedcard) && target->has_tag(tag_type::missedcard))
            return "ERROR_TARGET_MISSED";

        if (filter.check(target_card_filter::bronco) && !target->has_tag(tag_type::bronco))
            return "ERROR_TARGET_NOT_BRONCO";

        if (filter.check(target_card_filter::catbalou_panic) && !target->has_tag(tag_type::catbalou_panic))
            return "ERROR_TARGET_NOT_CATBALOU_PANIC";

        if (filter.check(target_card_filter::blue) && !target->is_blue())
            return "ERROR_TARGET_NOT_BLUE_CARD";

        if (filter.check(target_card_filter::train) && !target->is_train())
            return "ERROR_TARGET_NOT_TRAIN";

        if (filter.check(target_card_filter::blue_or_train) && !target->is_blue() && !target->is_train())
            return "ERROR_TARGET_NOT_BLUE_OR_TRAIN";

        if (filter.check(target_card_filter::black) != target->is_black())
            return "ERROR_TARGET_BLACK_CARD";

        if (filter.check(target_card_filter::hearts) && !target->sign.is_hearts())
            return "ERROR_TARGET_NOT_HEARTS";

        if (filter.check(target_card_filter::diamonds) && !target->sign.is_diamonds())
            return "ERROR_TARGET_NOT_DIAMONDS";

        if (filter.check(target_card_filter::clubs) && !target->sign.is_clubs())
            return "ERROR_TARGET_NOT_CLUBS";
        
        if (filter.check(target_card_filter::spades) && !target->sign.is_spades())
            return "ERROR_TARGET_NOT_SPADES";
        
        if (filter.check(target_card_filter::origin_card_suit)) {
            auto req = origin->m_game->top_request();
            card_ptr req_origin_card = req ? req->origin_card : nullptr;
            if (!req_origin_card) return "ERROR_NO_ORIGIN_CARD_SUIT";
            switch (req_origin_card->sign.suit) {
                case card_suit::hearts: if (!target->sign.is_hearts()) { return "ERROR_TARGET_NOT_HEARTS"; } break;
                case card_suit::diamonds: if (!target->sign.is_diamonds()) { return "ERROR_TARGET_NOT_DIAMONDS"; } break;
                case card_suit::clubs: if (!target->sign.is_clubs()) { return "ERROR_TARGET_NOT_CLUBS"; } break;
                case card_suit::spades: if (!target->sign.is_spades()) { return "ERROR_TARGET_NOT_SPADES"; } break;
                default: return "ERROR_NO_ORIGIN_CARD_SUIT";
            }
        }
        
        if (filter.check(target_card_filter::two_to_nine) && !target->sign.is_two_to_nine())
            return "ERROR_TARGET_NOT_TWO_TO_NINE";
        
        if (filter.check(target_card_filter::ten_to_ace) && !target->sign.is_ten_to_ace())
            return "ERROR_TARGET_NOT_TEN_TO_ACE";

        if (filter.check(target_card_filter::table) && target->pocket != pocket_type::player_table)
            return "ERROR_TARGET_NOT_TABLE_CARD";

        if (filter.check(target_card_filter::hand) && target->pocket != pocket_type::player_hand)
            return "ERROR_TARGET_NOT_HAND_CARD";

        return {};
    }
}