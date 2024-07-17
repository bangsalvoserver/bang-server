#include "messages.h"

namespace banggame {

    struct bot_settings {
        bool allow_timer_no_action;
        int max_random_tries;
        int bypass_prompt_after;
        std::vector<pocket_type> response_pockets;
        std::vector<pocket_type> in_play_pockets;
    };

    struct bot_info_t {
        std::vector<std::string> names;
        std::vector<utils::image_pixels> propics;
        bot_settings settings;
    };

    extern const bot_info_t bot_info;

}