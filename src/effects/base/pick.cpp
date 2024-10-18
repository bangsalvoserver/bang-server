#include "pick.h"

#include "game/game.h"
#include "game/possible_to_play.h"

#include "cards/filter_enums.h"

namespace banggame {

    bool effect_pick::can_play(card_ptr origin_card, player_ptr origin) {
        return origin->m_game->top_request<interface_picking>(origin) != nullptr;
    }

    prompt_string effect_pick::on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target) {
        return origin->m_game->top_request<interface_picking>()->pick_prompt(target);
    }

    void effect_pick::on_play(card_ptr origin_card, player_ptr origin, card_ptr target) {
        auto req = origin->m_game->top_request<interface_picking>();
        req->on_pick(target);
    }

    bool selection_picker::can_pick(const_card_ptr target_card) const {
        return target_card->pocket == pocket_type::selection;
    }

    bool interface_picking_player::in_target_set(const_player_ptr target_player) const {
        return target_player->alive() && can_pick(target_player);
    }

    bool effect_pick_player::can_play(card_ptr origin_card, player_ptr origin) {
        return origin->m_game->top_request<interface_picking_player>(origin) != nullptr;
    }

    prompt_string effect_pick_player::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        return origin->m_game->top_request<interface_picking_player>()->pick_prompt(target);
    }

    void effect_pick_player::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        auto req = origin->m_game->top_request<interface_picking_player>();
        req->on_pick(target);
    }

    void request_picking::auto_pick() {
        card_ptr only_card = get_single_element(get_all_playable_cards(target, true));
        if (only_card && only_card->has_tag(tag_type::pick)) {
            auto pick_cards = get_all_targetable_cards(target) | rv::filter([&](const_card_ptr c){ return in_target_set(c); });
            if (card_ptr target_card = get_single_element(pick_cards)) {
                on_pick(target_card);
            }
        }
    }

    void request_picking_player::auto_pick() {
        card_ptr only_card = get_single_element(get_all_playable_cards(target, true));
        if (only_card && only_card->has_tag(tag_type::pick)) {
            auto pick_players = target->m_game->m_players | rv::filter([&](const_player_ptr p){ return in_target_set(p); });
            if (player_ptr target_player = get_single_element(pick_players)) {
                on_pick(target_player);
            }
        }
    }

}