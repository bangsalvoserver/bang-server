#include "prompt_on_self_equip.h"

#include "game/game.h"

namespace banggame {
    
    game_string equip_prompt_on_self_equip::on_prompt(player *origin, card *target_card, player *target) {
        if (target == origin) {
            return {"PROMPT_EQUIP_ON_SELF", target_card};
        } else {
            return {};
        }
    }
}