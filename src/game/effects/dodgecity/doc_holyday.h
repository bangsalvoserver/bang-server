#ifndef __DODGECITY_DOC_HOLYDAY_H__
#define __DODGECITY_DOC_HOLYDAY_H__

#include "../card_effect.h"

namespace banggame {

    struct handler_doc_holyday {
        void on_play(card *origin_card, player *origin, tagged_value<target_type::cards> target_cards, player *target);
    };
}

#endif