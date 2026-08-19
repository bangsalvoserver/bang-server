#include "ruleset.h"
#include "game/expansion_set.h"
#include "effects/dodgecity/ruleset.h"
#include "game/game_table.h"

namespace banggame {

    bool ruleset_vlcata::is_valid_with(const expansion_set &set) {
        return set.contains(GET_RULESET(dodgecity));
    }

    void ruleset_vlcata::on_apply(game_ptr game) {
    }

}