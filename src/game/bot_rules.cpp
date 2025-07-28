#include "bot_rules.h"

#include "game_table.h"

#include "cards/filter_enums.h"

namespace banggame {

    bool rule_filter_by_pocket::operator()(card_node node) const {
        if (!node) return false;
        if (card_ptr choice_card = node->context.card_choice) {
            return choice_card->pocket == pocket;
        }
        return node->card->pocket == pocket;
    }

    bool rule_filter_by_pocket_not::operator()(card_node node) const {
        if (!node) return false;
        if (card_ptr choice_card = node->context.card_choice) {
            return choice_card->pocket != pocket;
        }
        return node->card->pocket != pocket;
    }

    bool rule_repeat::operator()(card_node node) const {
        return node && node->context.repeat_card != nullptr;
    }

    bool rule_tag_value::operator()(card_node node) const {
        return node && node->card->get_tag_value(tag) == std::optional{value};
    }

    bool rule_tag_value_not::operator()(card_node node) const {
        return node && node->card->get_tag_value(tag) != std::optional{value};
    }

    bool rule_do_nothing::operator()(card_node node) const {
        return node == nullptr;
    }
}