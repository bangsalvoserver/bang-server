#include "messages.h"

namespace banggame {

    struct bot_info_t {
        std::vector<std::string> names;
        std::vector<sdl::image_pixels> propics;
    };

    extern const bot_info_t bot_info;

}