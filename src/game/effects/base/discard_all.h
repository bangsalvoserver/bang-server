#ifndef __BASE_DISCARD_ALL_H__
#define __BASE_DISCARD_ALL_H__

#include "../card_effect.h"

namespace banggame {
    void queue_request_discard_all(player *target, bool death);
}

#endif