#ifndef __GIVE_CARD_H__
#define __GIVE_CARD_H__

#include "cards/card_fwd.h"

namespace banggame {

    bool give_card(player_ptr target, std::string_view card_name);

}

#endif