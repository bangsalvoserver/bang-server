#include "aim.h"

#include "game/game.h"
#include "cards/base/bang.h"

namespace banggame {
    
    game_string effect_aim::verify(card *origin_card, player *origin) {
        if (game_string error = effect_banglimit{}.verify(origin_card, origin))
            return error;

        auto is_bangcard = [origin](card *c) {
            return origin->is_bangcard(c) || c->has_tag(tag_type::play_as_bang);
        };

        constexpr effect_holder bang_holder{
            .player_filter{target_player_filter::reachable | target_player_filter::notself}
        };
        
        if ((std::ranges::none_of(origin->m_hand, is_bangcard)
         && std::ranges::none_of(origin->m_table, is_bangcard)
         && std::ranges::none_of(origin->m_characters, is_bangcard))

         || origin->make_player_target_set(origin_card, bang_holder).empty())
            return "ERROR_INVALID_MODIFIER_CARD";
        
        return {};
    }

    void effect_aim::on_play(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::apply_bang_modifier>(origin_card, [=](player *p, request_bang *req) {
            if (p == origin) {
                ++req->bang_damage;
                origin->m_game->remove_listeners(origin_card);
            }
        });
    }
}