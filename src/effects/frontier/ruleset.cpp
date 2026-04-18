#include "ruleset.h"

#include "game/game_table.h"

namespace banggame {

    void ruleset_frontier::on_apply(game_ptr game) {
        // TODO add_listener on_discard_pass
        // when discarding "HEAVY_GRUB" -> heal(3)

        // TODO add_listener on_destroy_card
        // when discarding "JACKALOPE" -> target draw(2)

        // TODO add_listener on_discard_pass (new event)
        // when discarding "FEUD" -> return error
    }
}