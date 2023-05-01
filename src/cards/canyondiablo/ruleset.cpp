#include "ruleset.h"

#include "game/game.h"

namespace banggame {
    void ruleset_canyondiablo::on_apply(game *game) {
        game->add_listener<event_type::check_damage_response>(nullptr, [=](player *target, bool &value) {
            if (!value && std::ranges::any_of(game->m_players, [target](player *p) {
                return p != target && p->alive() && !p->empty_hand();
            }) && !ranges::contains(game->m_discards, "SACRIFICE", &card::name)) {
                value = true;
            }
        });
        
        game->add_listener<event_type::on_equip_card>({nullptr, 5}, [](player *origin, player *target, card *origin_card, const effect_context &ctx) {
            if (origin_card->is_green() && !origin_card->inactive) {
                origin_card->inactive = true;
                origin->m_game->add_update<game_update_type::tap_card>(origin_card, true);
            }
        });
    }
}