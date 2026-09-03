#include "ruleset.h"
#include "game/expansion_set.h"
#include "game/game_table.h"

namespace banggame {

    bool ruleset_vlcata::is_valid_with(const expansion_set &set) {
        return true; // Umožňuje hrát Vlčata samostatně se základní hrou
    }

    void ruleset_vlcata::on_apply(game_ptr game) {
    }

}