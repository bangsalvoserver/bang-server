#include "filters.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"

#include "game.h"

namespace banggame::filters {

    bool is_player_bot(const player *origin) {
        return origin->is_bot();
    }

    game_string check_player_filter(const player *origin, target_player_filter filter, const player *target, const effect_context &ctx) {
        if (bool(filter & target_player_filter::dead)) {
            if (!bool(filter & target_player_filter::alive) && !target->check_player_flags(player_flags::dead)) {
                return "ERROR_TARGET_NOT_DEAD";
            }
        } else if (!target->alive()) {
            return "ERROR_TARGET_DEAD";
        }

        if (bool(filter & target_player_filter::self) && target != origin)
            return "ERROR_TARGET_NOT_SELF";

        if (bool(filter & target_player_filter::notself) && target == origin)
            return "ERROR_TARGET_SELF";
        
        if (bool(filter & target_player_filter::notorigin)) {
            auto req = origin->m_game->top_request();
            if (!req || req->origin == target) {
                return "ERROR_TARGET_ORIGIN";
            }
        }

        if (bool(filter & target_player_filter::notsheriff) && target->m_role == player_role::sheriff)
            return "ERROR_TARGET_SHERIFF";

        if (bool(filter & target_player_filter::not_empty_hand) && target->empty_hand())
            return "ERROR_TARGET_EMPTY_HAND";
        
        if (bool(filter & target_player_filter::not_empty_table) && target->empty_table())
            return "ERROR_TARGET_EMPTY_TABLE";

        if (bool(filter & target_player_filter::not_empty_cubes) && target->count_cubes() == 0)
            return "ERROR_TARGET_EMPTY_CUBES";
        
        if (bool(filter & target_player_filter::target_set)) {
            auto req = target->m_game->top_request<interface_target_set>(origin);
            if (!req || !req->in_target_set(target)) {
                return "ERROR_TARGET_NOT_IN_TARGET_SET";
            }
        }

        if (!ctx.ignore_distances && bool(filter & (target_player_filter::reachable | target_player_filter::range_1 | target_player_filter::range_2))) {
            int range = origin->get_range_mod();
            if (bool(filter & target_player_filter::reachable)) {
                int weapon_range = origin->get_weapon_range();
                if (weapon_range == 0) {
                    return "ERROR_TARGET_NOT_IN_RANGE";
                }
                range += weapon_range;
            } else if (bool(filter & target_player_filter::range_1)) {
                ++range;
            } else if (bool(filter & target_player_filter::range_2)) {
                range += 2;
            }
            if (origin->m_game->calc_distance(origin, target) > range) {
                return "ERROR_TARGET_NOT_IN_RANGE";
            }
        }

        return {};
    }

    bool is_equip_card(const card *target) {
        switch (target->pocket) {
        case pocket_type::player_hand:
        case pocket_type::shop_selection:
            return !target->is_brown();
        case pocket_type::train:
            return target->deck != card_deck_type::locomotive;
        default:
            return false;
        }
    }

    bool is_modifier_card(const player *origin, const card *origin_card) {
        return origin_card->get_modifier(origin->m_game->pending_requests()).type != nullptr;
    }

    bool is_bang_card(const player *origin, const card *target) {
        return origin->m_game->check_flags(game_flags::treat_any_as_bang)
            || origin->check_player_flags(player_flags::treat_any_as_bang)
            || target->has_tag(tag_type::bangcard)
            || origin->check_player_flags(player_flags::treat_missed_as_bang)
            && target->has_tag(tag_type::missed);
    }

    int get_card_cost(const card *target, bool is_response, const effect_context &ctx) {
        if (!is_response && !ctx.repeat_card && target->pocket != pocket_type::player_table) {
            if (ctx.card_choice) {
                target = ctx.card_choice;
            }
            return target->get_tag_value(tag_type::buy_cost).value_or(0) - ctx.discount;
        } else {
            return 0;
        }
    }

    game_string check_card_filter(const card *origin_card, const player *origin, target_card_filter filter, const card *target, const effect_context &ctx) {
        if (!bool(filter & target_card_filter::can_target_self) && target == origin_card)
            return "ERROR_TARGET_PLAYING_CARD";
        
        if (bool(filter & target_card_filter::cube_slot)) {
            if (target != target->owner->first_character() && !(target->is_orange() && target->pocket == pocket_type::player_table))
                return "ERROR_TARGET_NOT_CUBE_SLOT";
        } else if (target->deck == card_deck_type::character) {
            return "ERROR_TARGET_NOT_CARD";
        }

        if (bool(filter & target_card_filter::beer) && !target->has_tag(tag_type::beer))
            return "ERROR_TARGET_NOT_BEER";

        if (bool(filter & target_card_filter::bang) && !is_bang_card(origin, target))
            return "ERROR_TARGET_NOT_BANG";

        if (bool(filter & target_card_filter::bangcard) && !target->has_tag(tag_type::bangcard))
            return "ERROR_TARGET_NOT_BANG";

        if (bool(filter & target_card_filter::not_bangcard) && target->has_tag(tag_type::bangcard))
            return "ERROR_TARGET_BANG";

        if (bool(filter & target_card_filter::missed) && !target->has_tag(tag_type::missed))
            return "ERROR_TARGET_NOT_MISSED";

        if (bool(filter & target_card_filter::missedcard) && !target->has_tag(tag_type::missedcard))
            return "ERROR_TARGET_NOT_MISSED";

        if (bool(filter & target_card_filter::not_missedcard) && target->has_tag(tag_type::missedcard))
            return "ERROR_TARGET_MISSED";

        if (bool(filter & target_card_filter::bronco) && !target->has_tag(tag_type::bronco))
            return "ERROR_TARGET_NOT_BRONCO";

        if (bool(filter & target_card_filter::catbalou_panic)
            && !target->has_tag(tag_type::cat_balou)
            && !target->has_tag(tag_type::panic))
            return "ERROR_TARGET_NOT_CATBALOU_PANIC";

        if (bool(filter & target_card_filter::blue) && !target->is_blue())
            return "ERROR_TARGET_NOT_BLUE_CARD";

        if (bool(filter & target_card_filter::train) && !target->is_train())
            return "ERROR_TARGET_NOT_TRAIN";

        if (bool(filter & target_card_filter::nottrain) && target->is_train())
            return "ERROR_TARGET_TRAIN";

        if (bool(filter & target_card_filter::blue_or_train) && !target->is_blue() && !target->is_train())
            return "ERROR_TARGET_NOT_BLUE_OR_TRAIN";

        if (bool(filter & target_card_filter::black) != target->is_black())
            return "ERROR_TARGET_BLACK_CARD";

        if (bool(filter & target_card_filter::hearts) && !target->sign.is_hearts())
            return "ERROR_TARGET_NOT_HEARTS";

        if (bool(filter & target_card_filter::diamonds) && !target->sign.is_diamonds())
            return "ERROR_TARGET_NOT_DIAMONDS";

        if (bool(filter & target_card_filter::clubs) && !target->sign.is_clubs())
            return "ERROR_TARGET_NOT_CLUBS";
        
        if (bool(filter & target_card_filter::spades) && !target->sign.is_spades())
            return "ERROR_TARGET_NOT_SPADES";
        
        if (bool(filter & target_card_filter::origin_card_suit)) {
            auto req = origin->m_game->top_request();
            card *req_origin_card = req ? req->origin_card : nullptr;
            if (!req_origin_card) return "ERROR_NO_ORIGIN_CARD_SUIT";
            switch (req_origin_card->sign.suit) {
                case card_suit::hearts: if (!target->sign.is_hearts()) { return "ERROR_TARGET_NOT_HEARTS"; } break;
                case card_suit::diamonds: if (!target->sign.is_diamonds()) { return "ERROR_TARGET_NOT_DIAMONDS"; } break;
                case card_suit::clubs: if (!target->sign.is_clubs()) { return "ERROR_TARGET_NOT_CLUBS"; } break;
                case card_suit::spades: if (!target->sign.is_spades()) { return "ERROR_TARGET_NOT_SPADES"; } break;
                default: return "ERROR_NO_ORIGIN_CARD_SUIT";
            }
        }
        
        if (bool(filter & target_card_filter::two_to_nine) && !target->sign.is_two_to_nine())
            return "ERROR_TARGET_NOT_TWO_TO_NINE";
        
        if (bool(filter & target_card_filter::ten_to_ace) && !target->sign.is_ten_to_ace())
            return "ERROR_TARGET_NOT_TEN_TO_ACE";

        if (bool(filter & target_card_filter::selection) && target->pocket != pocket_type::selection)
            return "ERROR_TARGET_NOT_SELECTION";

        if (bool(filter & target_card_filter::table) && target->pocket != pocket_type::player_table)
            return "ERROR_TARGET_NOT_TABLE_CARD";

        if (bool(filter & target_card_filter::hand) && target->pocket != pocket_type::player_hand)
            return "ERROR_TARGET_NOT_HAND_CARD";

        return {};
    }
}