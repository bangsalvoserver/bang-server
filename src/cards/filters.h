#ifndef __FILTERS_H__
#define __FILTERS_H__

#include "filters_simple.h"

#include "game/game_string.h"

namespace banggame::filters {

    const serial::card_list &get_selected_cubes(const card *origin_card, const effect_context &ctx);

    game_string check_player_filter(const player *origin, target_player_filter filter, const player *target, const effect_context &ctx = {});

    int get_card_cost(const card *target, bool is_response, const effect_context &ctx);

    game_string check_card_filter(const card *origin_card, const player *origin, target_card_filter filter, const card *target, const effect_context &ctx = {});
}

#endif