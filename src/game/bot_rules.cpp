#include "bot_rules.h"

#include "game_table.h"

#include "effects/base/card_choice.h"

#include "cards/filter_enums.h"

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

    bool rule_do_nothing::operator()(card_node node) const {
        return node == nullptr;
    }
}