#include "pick.h"

#include "game/game.h"
#include "game/possible_to_play.h"

#include "cards/filter_enums.h"

namespace banggame {

    game_string effect_pick::get_error(card_ptr origin_card, player_ptr origin, card_ptr target) {
        if (auto req = origin->m_game->top_request<interface_picking>(origin)) {
            return {};
        }
        return "ERROR_INVALID_ACTION";
    }

    game_string effect_pick::on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target) {
        return origin->m_game->top_request<interface_picking>()->pick_prompt(target);
    }

    void effect_pick::on_play(card_ptr origin_card, player_ptr origin, card_ptr target) {
        auto req = origin->m_game->top_request<interface_picking>();
        req->on_pick(target);
    }

    bool selection_picker::can_pick(const_card_ptr target_card) const {
        return target_card->pocket == pocket_type::selection;
    }

    void request_picking::auto_pick() {
        card_ptr only_card = get_single_element(get_all_playable_cards(target, true));
        if (only_card && only_card->has_tag(tag_type::pick)) {
            auto pick_cards = get_all_targetable_cards(target) | rv::filter(std::bind_front(&request_picking::can_pick, this));
            if (card_ptr target_card = get_single_element(pick_cards)) {
                on_pick(target_card);
            }
        }
    }

}