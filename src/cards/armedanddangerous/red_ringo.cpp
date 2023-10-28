#include "red_ringo.h"

#include "game/game.h"

namespace banggame {
    
    void equip_red_ringo::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_game_setup>({target_card, 4}, [=](player *origin) {
            target->m_game->add_cubes(target->first_character(), max_cubes);
        });
    }

    game_string handler_red_ringo::get_error(card *origin_card, player *origin, const effect_target_list &targets) {
        if (origin->first_character()->num_cubes < targets.size()) {
            return {"ERROR_NOT_ENOUGH_CUBES_ON", origin_card};
        }
        auto target_cards = targets | std::views::transform([](const effect_target_pair &target) -> card * {
            return target.target.get<target_type::card>();
        });
        if (auto it = std::ranges::find_if(target_cards, [&](card *target_card) {
            return int(std::ranges::count(target_cards, target_card)) + target_card->num_cubes > max_cubes;
        }); it != target_cards.end()) {
            return {"ERROR_CARD_HAS_FULL_CUBES", *it};
        }
        return {};
    }

    void handler_red_ringo::on_play(card *origin_card, player *origin, const effect_target_list &targets) {
        for (const effect_target_pair &target : targets) {
            card *target_card = target.target.get<target_type::card>();
            origin->m_game->move_cubes(origin->first_character(), target_card, 1);
        }
    }
}