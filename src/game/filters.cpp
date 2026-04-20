#include "filters.h"

#include "effects/armedanddangerous/belltower.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"

#include "game_table.h"

namespace banggame {

    static bool check_distance(const_player_ptr origin, const_player_ptr target, const effect_context &ctx, int range) {
        return range != 0 && (
            ctx.contains<contexts::ignore_distances>()
            || origin->check_player_flags(player_flag::ignore_distances)
            || origin->m_game->calc_distance(origin, target) <= (origin->get_range_mod() + range)
        );
    }

    game_string check_player_filter(const_card_ptr origin_card, const_player_ptr origin, player_filter_bitset filter, player_ptr target, const effect_context &ctx) {
        for (target_player_filter value : filter) {
            switch (value) {
            case target_player_filter::alive:
                if (target->alive()) continue;
                return {"ERROR_TARGET_DEAD", origin_card, target};

            case target_player_filter::dead:
                if (!target->alive()) continue;
                return {"ERROR_TARGET_NOT_DEAD", origin_card, target};

            case target_player_filter::self:
                if (target == origin) continue;
                return {"ERROR_TARGET_NOT_SELF", origin_card, target};

            case target_player_filter::notself:
                if (target != origin) continue;
                return {"ERROR_TARGET_SELF", origin_card, target};

            case target_player_filter::notsheriff:
                if (!target->is_sheriff()) continue;
                return {"ERROR_TARGET_SHERIFF", origin_card, target};

            case target_player_filter::not_empty:
                if (!target->empty_hand() || !target->empty_table()) continue;
                return {"ERROR_TARGET_EMPTY", origin_card, target};

            case target_player_filter::not_empty_hand:
                if (!target->empty_hand()) continue;
                return {"ERROR_TARGET_EMPTY_HAND", origin_card, target};

            case target_player_filter::not_empty_table:
                if (!target->empty_table()) continue;
                return {"ERROR_TARGET_EMPTY_TABLE", origin_card, target};

            case target_player_filter::not_empty_cubes:
                if (count_cubes(target) != 0) continue;
                return {"ERROR_TARGET_EMPTY_CUBES", origin_card, target};

            case target_player_filter::reachable:
                if (check_distance(origin, target, ctx, origin->get_weapon_range())) continue;
                return {"ERROR_TARGET_NOT_IN_RANGE", origin_card, target};

            case target_player_filter::range_1:
                if (check_distance(origin, target, ctx, 1)) continue;
                return {"ERROR_TARGET_NOT_IN_RANGE", origin_card, target};

            case target_player_filter::range_2:
                if (check_distance(origin, target, ctx, 2)) continue;
                return {"ERROR_TARGET_NOT_IN_RANGE", origin_card, target};

            case target_player_filter::notorigin:
                if (auto req = origin->m_game->top_request())
                    if (req->origin != target) continue;
                return {"ERROR_TARGET_ORIGIN", origin_card, target};

            case target_player_filter::target_set:
                if (auto req = origin->m_game->top_request<interface_target_set_players>(target_is{origin}))
                    if (req->in_target_set(target)) continue;
                return {"ERROR_TARGET_NOT_IN_TARGET_SET", origin_card, target};
            }
        }

        return {};
    }

    game_string check_card_filter(const_card_ptr origin_card, const_player_ptr origin, card_filter_bitset filter, card_ptr target, const effect_context &ctx) {
        if (target == origin_card) {
            return {"ERROR_TARGET_PLAYING_CARD", origin_card, target};
        }

        if (!filter.check(target_card_filter::target_set)) {
            if (filter.check(target_card_filter::selection) != (target->pocket == pocket_type::selection))
                return {"ERROR_TARGET_NOT_SELECTION", origin_card, target};
            
            switch (target->pocket) {
            case pocket_type::player_hand:
            case pocket_type::player_table:
            case pocket_type::selection:
                break;
            default:
                return {"ERROR_CARD_NOT_TARGETABLE", origin_card, target};
            }
        }

        if (filter.check(target_card_filter::black) != target->is_black())
            return {"ERROR_TARGET_BLACK_CARD", origin_card, target};

        for (target_card_filter value : filter) {
            switch (value) {
            case target_card_filter::bang:
                if (target->is_bang_card(origin)) continue;
                return {"ERROR_TARGET_NOT_BANG", origin_card, target};

            case target_card_filter::used_bang:
                if (origin->m_game->check_flags(game_flag::showdown) || target->is_bang_card(origin)) continue;
                return {"ERROR_TARGET_NOT_BANG", origin_card, target};

            case target_card_filter::bangcard:
                if (target->has_tag(tag_type::bangcard)) continue;
                return {"ERROR_TARGET_NOT_BANG", origin_card, target};

            case target_card_filter::not_bangcard:
                if (!target->has_tag(tag_type::bangcard)) continue;
                return {"ERROR_TARGET_BANG", origin_card, target};

            case target_card_filter::missed:
                if (target->has_tag(tag_type::missed)) continue;
                return {"ERROR_TARGET_NOT_MISSED", origin_card, target};

            case target_card_filter::missedcard:
                if (target->has_tag(tag_type::missedcard)) continue;
                return {"ERROR_TARGET_NOT_MISSED", origin_card, target};

            case target_card_filter::not_missedcard:
                if (!target->has_tag(tag_type::missedcard)) continue;
                return {"ERROR_TARGET_MISSED", origin_card, target};

            case target_card_filter::bronco:
                if (target->has_tag(tag_type::bronco)) continue;
                return {"ERROR_TARGET_NOT_BRONCO", origin_card, target};

            case target_card_filter::catbalou_panic:
                if (target->has_tag(tag_type::catbalou_panic)) continue;
                return {"ERROR_TARGET_NOT_CATBALOU_PANIC", origin_card, target};

            case target_card_filter::beer:
                if (target->has_tag(tag_type::beer)) continue;
                return {"ERROR_TARGET_NOT_BEER", origin_card, target};

            case target_card_filter::blue:
                if (target->is_blue()) continue;
                return {"ERROR_TARGET_NOT_BLUE_CARD", origin_card, target};

            case target_card_filter::train:
                if (target->is_train()) continue;
                return {"ERROR_TARGET_NOT_TRAIN", origin_card, target};

            case target_card_filter::blue_or_train:
                if (target->is_blue() || target->is_train()) continue;
                return {"ERROR_TARGET_NOT_BLUE_OR_TRAIN", origin_card, target};

            case target_card_filter::hearts:
                if (target->sign.is_hearts()) continue;
                return {"ERROR_TARGET_NOT_HEARTS", origin_card, target};

            case target_card_filter::diamonds:
                if (target->sign.is_diamonds()) continue;
                return {"ERROR_TARGET_NOT_DIAMONDS", origin_card, target};

            case target_card_filter::clubs:
                if (target->sign.is_clubs()) continue;
                return {"ERROR_TARGET_NOT_CLUBS", origin_card, target};

            case target_card_filter::spades:
                if (target->sign.is_spades()) continue;
                return {"ERROR_TARGET_NOT_SPADES", origin_card, target};

            case target_card_filter::two_to_nine:
                if (target->sign.is_two_to_nine()) continue;
                return {"ERROR_TARGET_NOT_TWO_TO_NINE", origin_card, target};

            case target_card_filter::ten_to_ace:
                if (target->sign.is_ten_to_ace()) continue;
                return {"ERROR_TARGET_NOT_TEN_TO_ACE", origin_card, target};

            case target_card_filter::table:
                if (target->pocket == pocket_type::player_table) continue;
                return {"ERROR_TARGET_NOT_TABLE_CARD", origin_card, target};

            case target_card_filter::hand:
                if (target->pocket == pocket_type::player_hand) continue;
                return {"ERROR_TARGET_NOT_HAND_CARD", origin_card, target};

            case target_card_filter::not_self_hand:
                if (target->pocket != pocket_type::player_hand || target->owner != origin) continue;
                return {"ERROR_TARGET_SELF_HAND", origin_card, target};

            case target_card_filter::target_set:
                if (auto req = origin->m_game->top_request<interface_target_set_cards>(target_is{origin}))
                    if (req->in_target_set(target)) continue;
                return {"ERROR_TARGET_NOT_IN_TARGET_SET", origin_card, target};

            case target_card_filter::origin_card_suit:
                if (auto req = origin->m_game->top_request())
                    if (req->origin_card && req->origin_card->sign.suit == target->sign.suit) continue;
                return {"ERROR_NO_ORIGIN_CARD_SUIT", origin_card, target};
            }
        }

        return {};
    }
}