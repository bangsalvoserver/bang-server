#include "dynamite.h"

#include "game/game_table.h"

#include "predraw_check.h"

namespace banggame {
    void equip_dynamite::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_predraw_check>(target_card, [=](player_ptr e_player, card_ptr e_card) {
            if (e_player == target && e_card == target_card) {
                target->m_game->queue_request<request_check>(target, target_card, [](card_sign sign) {
                    return !sign.is_spades() || !sign.is_two_to_nine();
                }, [=](bool result) {
                    if (!result) {
                        target->m_game->add_log("LOG_CARD_EXPLODES", target_card);
                        target->m_game->play_sound("dynamite");
                        target->discard_card(target_card);
                        target->damage(target_card, nullptr, 3);
                    } else {
                        for (player_ptr dest : target->m_game->range_other_players(target)) {
                            if (!dest->find_equipped_card(target_card)) {
                                target->disable_equip(target_card);
                                dest->equip_card(target_card);
                                break;
                            }
                        }
                    }
                });
            }
        });
    }
}