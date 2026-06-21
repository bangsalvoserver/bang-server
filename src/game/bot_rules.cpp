#include "bot_rules.h"

#include "game_table.h"

#include "effects/base/card_choice.h"

#include "cards/filter_enums.h"

namespace banggame {

    bool rule_filter_by_pocket::operator()(card_node node) const {
        if (!node) return false;
        if (card_ptr choice_card = node->context.get<contexts::card_choice>()) {
            return choice_card->pocket == pocket;
        }
        return node->card->pocket == pocket;
    }

    bool rule_filter_by_pocket_not::operator()(card_node node) const {
        if (!node) return false;
        if (card_ptr choice_card = node->context.get<contexts::card_choice>()) {
            return choice_card->pocket != pocket;
        }
        return node->card->pocket != pocket;
    }

    bool rule_equip::operator()(card_node node) const {
        return node && node->card->is_equip_card();
    }

    bool rule_repeat::operator()(card_node node) const {
        return node && node->context.contains<contexts::repeat_card>();
    }

    bool rule_tag_value::operator()(card_node node) const {
        if (node) {
            std::optional<tag_int> card_tag = node->card->get_tag_value(tag);
            if (value.has_value()) {
                return card_tag == value;
            } else {
                return card_tag.has_value();
            }
        }
        return false;
    }

    bool rule_tag_value_not::operator()(card_node node) const {
        if (node) {
            std::optional<tag_int> card_tag = node->card->get_tag_value(tag);
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