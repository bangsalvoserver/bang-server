#ifndef __BOT_AI_H__
#define __BOT_AI_H__

#include "cards/card_serial.h"

namespace banggame::bot_ai {
    bool respond_to_request(player *origin);
    bool play_in_turn(player *origin);
}

#endif