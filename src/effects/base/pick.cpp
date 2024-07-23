#include "pick.h"

#include "game/game.h"
#include "game/possible_to_play.h"

#include "cards/filter_enums.h"

namespace banggame {

    game_string effect_pick::get_error(card *origin_card, player *origin, card *target) {
        if (auto req = origin->m_game->top_request<interface_picking>(origin)) {
            return {};
        }
        return "ERROR_INVALID_ACTION";
    }

    game_string effect_pick::on_prompt(card *origin_card, player *origin, card *target) {
        return origin->m_game->top_request<interface_picking>()->pick_prompt(target);
    }

    void effect_pick::on_play(card *origin_card, player *origin, card *target) {
        auto req = origin->m_game->top_request<interface_picking>();
        req->on_pick(target);
    }

    bool selection_picker::can_pick(const card *target_card) const {
        return target_card->pocket == pocket_type::selection;
    }

    void request_picking::auto_pick() {
        card *only_card = get_single_element(get_all_playable_cards(target, true));
        if (only_card && only_card->has_tag(tag_type::pick)) {
            if (card *target_card = get_single_element(get_request_target_set_cards(target))) {
                on_pick(target_card);
            }
        }
    }

}