#include "bot_rules.h"

#include "game_table.h"

#include "effects/base/card_choice.h"
#include "effects/base/draw.h"
#include "effects/base/resolve.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"

namespace banggame {

    static card_ptr get_first_card(card_node node) {
        if (node) {
            return node->modifiers.empty() ? node->card : node->modifiers.front().card;
        }
        return nullptr;
    }

    bool rule_filter_by_pocket::operator()(card_node node) const {
        if (card_ptr origin_card = get_first_card(node)) {
            return origin_card->pocket == pocket;
        }
        return false;
    }

    bool rule_filter_by_pocket_not::operator()(card_node node) const {
        if (card_ptr origin_card = get_first_card(node)) {
            return origin_card->pocket != pocket;
        }
        return false;
    }

    bool rule_equip::operator()(card_node node) const {
        return node && node->card->is_equip_card();
    }

    bool rule_tag_value::operator()(card_node node) const {
        if (card_ptr origin_card = get_first_card(node)) {
            std::optional<tag_int> card_tag = origin_card->get_tag_value(tag);
            if (value.has_value()) {
                return card_tag == value;
            } else {
                return card_tag.has_value();
            }
        }
        return false;
    }

    bool rule_tag_value_not::operator()(card_node node) const {
        if (card_ptr origin_card = get_first_card(node)) {
            std::optional<tag_int> card_tag = origin_card->get_tag_value(tag);
            if (value.has_value()) {
                return card_tag != value;
            } else {
                return !card_tag.has_value();
            }
        }
        return false;
    }

    bool rule_do_something::operator()(card_node node) const {
        return node != nullptr;
    }

    bool condition_drawing_or_dismissable::operator()(player_ptr origin) const {
        if (auto req = origin->m_game->top_request<request_draw>(target_is{origin})) {
            return true;
        }
        if (auto req = origin->m_game->top_request<interface_resolvable>(target_is{origin})) {
            return req->get_resolve_type() == resolve_type::dismiss;
        }
        return false;
    }

    bool condition_sheriff_first_turn::operator()(player_ptr origin) const {
        return origin->is_sheriff()
            && !origin->m_game->pending_requests()
            && origin->m_hand.size() <= origin->max_cards_end_of_turn()
            && rn::all_of(origin->m_game->m_players, [&](player_ptr target) {
                return origin == target || !target->alive() || !target->m_player_flags.check_any({
                    player_flag::role_revealed, player_flag::positive_karma, player_flag::negative_karma
                });
            });
    }

    bool condition_response::operator()(player_ptr origin) const {
        return origin->m_game->pending_requests();
    }

    bool condition_in_play::operator()(player_ptr origin) const {
        return !origin->m_game->pending_requests();
    }
}