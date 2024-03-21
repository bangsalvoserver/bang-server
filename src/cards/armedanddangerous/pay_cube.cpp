#include "pay_cube.h"

#include "cards/base/heal.h"
#include "cards/filters.h"

#include "game/game.h"

namespace banggame {

    void effect_pay_cube::add_context(card *origin_card, player *origin, card *target, effect_context &ctx) {
        auto it = rn::find_if(ctx.selected_cubes, [&](const card_cubes_pair &pair) { return pair.card == origin_card; });
        if (it != ctx.selected_cubes.end()) {
            it->cubes.push_back(target);
        } else {
            ctx.selected_cubes.emplace_back(card_cubes_pair{origin_card, serial::card_list{target}});
        }
    }
    
    struct card_cube_ordering {
        bool operator()(card *lhs, card *rhs) const {
            if (lhs->pocket == pocket_type::player_table && rhs->pocket == pocket_type::player_table) {
                return rn::find(lhs->owner->m_table, lhs) < rn::find(rhs->owner->m_table, rhs);
            } else {
                return lhs->pocket == pocket_type::player_character;
            }
        }
    };

    void effect_pay_cube::on_play(card *origin_card, player *origin, const effect_context &ctx) {
        std::map<card *, int, card_cube_ordering> card_cube_map;
        for (card *cube : filters::get_selected_cubes(origin_card, ctx)) {
            ++card_cube_map[cube];
        }
        for (const auto &[c, ncubes] : card_cube_map) {
            origin->m_game->move_cubes(c, nullptr, ncubes);
        }
    }

}