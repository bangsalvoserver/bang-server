#ifndef __GAME_BOT_RULE_H__
#define __GAME_BOT_RULE_H__

#include "cards/card_defs.h"
#include "cards/filter_enums.h"

#include "utils/fixed_string.h"

namespace banggame {
    struct playable_card_info;
    using card_node = const playable_card_info *;

    class bot_rule {
    private:
        const void *data;
        bool (*thunk)(const void *, card_node);
    
    public:
        template<std::predicate<card_node> Function>
        bot_rule(const Function &fn)
            : data{&fn}
            , thunk{[](const void *data, card_node node) {
                return std::invoke(*static_cast<const Function *>(data), node);
            }} {}
    
        bool operator()(card_node node) const {
            return (*thunk)(data, node);
        }
    };

    template<utils::fixed_string Name>
    struct bot_rule_map;

    #define DEFINE_BOT_RULE(name, rule_type) template<> struct bot_rule_map<#name> { using type = rule_type; };

    #define BOT_RULE(name) bot_rule_map<#name>::type
    
    struct rule_filter_by_pocket {
        pocket_type pocket;
        bool operator()(card_node) const;
    };

    DEFINE_BOT_RULE(pocket, rule_filter_by_pocket)

    struct rule_filter_by_pocket_not {
        pocket_type pocket;
        bool operator()(card_node) const;
    };

    DEFINE_BOT_RULE(pocket_not, rule_filter_by_pocket_not)

    struct rule_equip {
        bool operator()(card_node) const;
    };

    DEFINE_BOT_RULE(equip, rule_equip)

    struct rule_repeat {
        bool operator()(card_node) const;
    };

    DEFINE_BOT_RULE(repeat, rule_repeat)

    struct rule_tag_value {
        tag_type tag;
        tag_int value;
        bool operator()(card_node) const;
    };

    DEFINE_BOT_RULE(tag_value, rule_tag_value)

    struct rule_tag_value_not {
        tag_type tag;
        tag_int value;
        bool operator()(card_node) const;
    };

    DEFINE_BOT_RULE(tag_value_not, rule_tag_value_not)

    struct rule_do_nothing {
        bool operator()(card_node) const;
    };

    DEFINE_BOT_RULE(do_nothing, rule_do_nothing)
}

#endif