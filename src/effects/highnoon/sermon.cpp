#include "sermon.h"

#include "cards/filter_enums.h"
#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    static game_string get_sermon_error(card_ptr target_card, player_ptr origin, card_ptr origin_card) {
        if (origin == origin->m_game->m_playing
            && (!origin_card->owner || origin_card->owner == origin)
            && origin_card->has_tag(origin_card->pocket == pocket_type::player_hand ? tag_type::bangcard : tag_type::play_as_bang)
        ) {
            return {"ERROR_CARD_DISABLED_BY", origin_card, target_card};
        }
        return {};
    }

    void equip_sermon::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::check_play_card>(target_card, [=](player_ptr origin, card_ptr origin_card, const effect_context &ctx)  {
            return get_sermon_error(target_card, origin, origin_card);
        });

        target->m_game->add_listener<event_type::check_use_card>(target_card, [=](player_ptr origin, card_ptr origin_card) {
            return get_sermon_error(target_card, origin, origin_card);
        });
    }

}