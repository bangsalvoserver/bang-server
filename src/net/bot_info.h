#ifndef __BOT_INFO_H__
#define __BOT_INFO_H__

#include "messages.h"

#include "game/bot_rules.h"

#include "utils/fixed_string.h"

#include "image_registry.h"

namespace banggame {

    using bot_rule_list = std::span<const bot_rule>;

    // A player-state condition selects which ordered rule list applies.
    struct bot_conditional_rules {
        bot_condition condition;
        bot_rule_list rules;
    };

    using bot_conditional_rule_list = std::span<const bot_conditional_rules>;

    struct bot_settings {
        int max_retries;
        int bypass_retries;
        // Checked in order; bot AI uses the rules from the first matching condition.
        bot_conditional_rule_list rules;
    };

    struct bot_info_t {
        uint32_t propic_size;
        std::span<const std::string_view> names;
        std::span<const image_registry::registered_image> propics;
        bot_settings settings;
    };

    extern const bot_info_t bot_info;

}

#endif
