#ifndef __BOT_INFO_H__
#define __BOT_INFO_H__

#include "messages.h"

#include "game/bot_rules.h"

#include "utils/fixed_string.h"

#include "image_registry.h"

namespace banggame {

    struct bot_settings {
        bool allow_timer_no_action;
        int max_random_tries;
        int bypass_prompt_after;
        std::initializer_list<bot_rule> response_rules;
        std::initializer_list<bot_rule> in_play_rules;
    };

    struct bot_info_t {
        uint32_t propic_size;
        std::span<std::string_view> names;
        std::initializer_list<image_registry::registered_image> propics;
        bot_settings settings;
    };

    extern const bot_info_t bot_info;

}

#endif