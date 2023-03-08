#include "ghosttown.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    void equip_ghosttown::on_enable(card *target_card, player *origin) {
        origin->m_game->add_listener<event_type::check_revivers>(target_card, [=](player *target) {
            if (!target->alive()) {
                origin->m_game->add_log("LOG_REVIVE", target, target_card);
                target->add_player_flags(player_flags::ghost_town);
                for (auto *c : target->m_characters) {
                    target->enable_equip(c);
                }
            }
        });
        origin->m_game->add_listener<event_type::on_turn_end>({target_card, -3}, [](player *target, bool skipped) {
            if (target->check_player_flags(player_flags::ghost_town)) {
                target->m_game->queue_action([=]{
                    if (target->m_extra_turns == 0 && target->remove_player_flags(player_flags::ghost_town) && !target->alive()) {
                        target->m_game->handle_player_death(nullptr, target, discard_all_reason::disable_ghost_town);
                    }
                });
            }
        });
        origin->m_game->add_listener<event_type::count_cards_to_draw>({target_card, 1}, [](player *target, int &value) {
            if (target->check_player_flags(player_flags::ghost_town)) {
                ++value;
            }
        });
    }
}