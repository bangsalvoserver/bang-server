#ifndef __GAME_BOT_RULE_H__
#define __GAME_BOT_RULE_H__

#include "cards/card_defs.h"

#include "cards/filter_enums.h"

#include <functional>

namespace banggame {
    struct playable_card_info;
    using card_node = const playable_card_info *;

    using bot_rule = std::move_only_function<bool(card_node) const>;

    template<utils::fixed_string Name>
    struct bot_rule_function_map;

    #define DEFINE_BOT_RULE(name, function, ...) \
        bot_rule function(__VA_ARGS__); \
        template<> struct bot_rule_function_map<#name> { \
            static constexpr auto value = function; \
        };

    #define BUILD_BOT_RULE(name, ...) \
        bot_rule_function_map<#name>::value(__VA_ARGS__)

    DEFINE_BOT_RULE(pocket, rule_filter_by_pocket, pocket_type)
    DEFINE_BOT_RULE(pocket_not, rule_filter_by_pocket_not, pocket_type)
    DEFINE_BOT_RULE(repeat, rule_repeat)
    DEFINE_BOT_RULE(preselect, rule_preselect)
    DEFINE_BOT_RULE(tag_value, rule_tag_value, tag_type, int)
    DEFINE_BOT_RULE(tag_value_not, rule_tag_value_not, tag_type, int)
    DEFINE_BOT_RULE(do_nothing, rule_do_nothing)
}

#endif