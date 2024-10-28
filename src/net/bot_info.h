#ifndef __BOT_INFO_H__
#define __BOT_INFO_H__

#include "messages.h"

#include "cards/card_defs.h"

#include "utils/fixed_string.h"

#include "image_registry.h"

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

    struct bot_settings {
        bool allow_timer_no_action;
        int max_random_tries;
        int bypass_prompt_after;
        std::initializer_list<bot_rule> response_rules;
        std::initializer_list<bot_rule> in_play_rules;
    };

    struct bot_info_t {
        uint32_t propic_size;
        std::initializer_list<std::string> names;
        std::initializer_list<image_registry::registered_image> propics;
        bot_settings settings;
    };

    extern const bot_info_t bot_info;

    DEFINE_BOT_RULE(pocket, rule_filter_by_pocket, pocket_type)
    DEFINE_BOT_RULE(repeat, rule_repeat)

}

#endif