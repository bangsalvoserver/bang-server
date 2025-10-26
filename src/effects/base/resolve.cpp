#include "resolve.h"

#include "game/game_table.h"
#include "game/game_options.h"
#include "game/possible_to_play.h"

#include "cards/filter_enums.h"

namespace banggame {

    prompt_string effect_resolve::on_prompt(card_ptr origin_card, player_ptr origin) {
        return origin->m_game->top_request<interface_resolvable>()->resolve_prompt();
    }

    bool effect_resolve::can_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        if (!ctx.temp_missable) {
            if (auto req = origin->m_game->top_request<interface_resolvable>(target_is{origin})) {
                return req->get_resolve_type() == type;
            }
        }
        return false;
    }
    
    void effect_resolve::on_play(card_ptr origin_card, player_ptr origin) {
        auto req = origin->m_game->top_request<interface_resolvable>();
        req->on_resolve();
    }

    bool request_resolvable::auto_resolvable() const {
        card_ptr only_card = get_single_element(get_all_playable_cards(target, true));
        return only_card && only_card->has_tag(tag_type::resolve);
    }
    
    void request_resolvable::auto_resolve() {
        if (auto_resolvable()) {
            on_resolve();
        }
    }
    
    void request_auto_resolvable::auto_resolve() {
        if (auto_resolvable()) {
            set_duration(target->m_game->m_options.auto_resolve_timer);
        }
    }
}