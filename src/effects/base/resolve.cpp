#include "resolve.h"

#include "game/game.h"
#include "game/possible_to_play.h"

#include "cards/filter_enums.h"

namespace banggame {

    bool effect_resolve::can_play(card *origin_card, player *origin) {
        return origin->m_game->top_request<interface_resolvable>(origin) != nullptr;
    }
    
    void effect_resolve::on_play(card *origin_card, player *origin) {
        auto req = origin->m_game->top_request<interface_resolvable>();
        req->on_resolve();
    }
    
    void request_resolvable::auto_resolve() {
        card *only_card = get_single_element(get_all_playable_cards(target, true));
        if (only_card && only_card->has_tag(tag_type::resolve)) {
            on_resolve();
        }
    }
}