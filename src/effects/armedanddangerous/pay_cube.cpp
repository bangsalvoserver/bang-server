#include "pay_cube.h"

#include "effects/base/heal.h"

#include "game/game.h"

namespace banggame {

    void effect_pay_cube::add_context(card *origin_card, player *origin, card *target, effect_context &ctx) {
        ctx.selected_cubes.insert(origin_card, target);
    }
    
    struct card_cube_ordering {
        bool operator()(card *lhs, card *rhs) const {
            if (lhs->pocket == pocket_type::player_table && rhs->pocket == pocket_type::player_table) {
                return rn::find(lhs->owner->m_table, lhs) < rn::find(rhs->owner->m_table, rhs);
            } else {
                return rhs->pocket == pocket_type::player_table;
            }
        }
    };

    void effect_pay_cube::on_play(card *origin_card, player *origin, const effect_context &ctx) {
        std::map<card *, int, card_cube_ordering> card_cube_map;
        for (card *cube : ctx.selected_cubes[origin_card]) {
            ++card_cube_map[cube];
        }
        for (const auto &[c, ncubes] : card_cube_map) {
            c->move_cubes(nullptr, ncubes);
        }
    }

}