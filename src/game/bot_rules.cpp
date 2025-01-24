#include "bot_rules.h"

#include "game_table.h"

#include "cards/filter_enums.h"

namespace banggame {

    bot_rule rule_filter_by_pocket(pocket_type pocket) {
        return [=](card_node node) {
            if (!node) return false;
            if (card_ptr choice_card = node->context.get().card_choice) {
                return choice_card->pocket == pocket;
            }
            return node->card->pocket == pocket;
        };
    }

    bot_rule rule_filter_by_pocket_not(pocket_type pocket) {
        return [=](card_node node) {
            if (!node) return false;
            if (card_ptr choice_card = node->context.get().card_choice) {
                return choice_card->pocket != pocket;
            }
            return node->card->pocket != pocket;
        };
    }

    bot_rule rule_repeat() {
        return [](card_node node) {
            return node && node->context.get().repeat_card != nullptr;
        };
    }

    bot_rule rule_tag_value(tag_type type, int value) {
        return [=](card_node node) {
            return node && node->card->get_tag_value(type) == std::optional{value};
        };
    }

    bot_rule rule_tag_value_not(tag_type type, int value) {
        return [=](card_node node) {
            return node && node->card->get_tag_value(type) != std::optional{value};
        };
    }

    bot_rule rule_do_nothing() {
        return [](card_node node) {
            return node == nullptr;
        };
    }
}