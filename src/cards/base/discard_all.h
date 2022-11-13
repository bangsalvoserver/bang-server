#ifndef __BASE_DISCARD_ALL_H__
#define __BASE_DISCARD_ALL_H__

#include "cards/card_effect.h"

namespace banggame {
    enum class discard_all_reason {
        death,
        sheriff_killed_deputy,
        disable_temp_ghost,
        discard_ghost
    };

    void queue_request_discard_all(player *target, discard_all_reason reason);
}

#endif