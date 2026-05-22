#include "ray_owe.h"

#include "ruleset.h"

#include "cards/game_events.h"

#include "effects/base/steal_destroy.h"

#include "game/game_table.h"

namespace banggame {

    void equip_ray_owe::on_enable(card_ptr origin_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_destroy_card>(origin_card, [=](player_ptr origin, card_ptr e_origin_card, card_ptr target_card, bool is_destroyed, destroy_flags &flags) {
            if (origin == target && e_origin_card == origin_card) {
                event_card_key key{origin_card, 2};

                pocket_type pocket = target_card->pocket;
                player_ptr owner = target_card->owner;

                int num_cubes = pocket == pocket_type::player_table && target_card->is_orange() ? target_card->num_cubes() : 0;
                player_ptr tracked_player = pocket == pocket_type::player_table && target_card->is_purple() ? get_tracked_player(target_card) : nullptr;

                auto return_to_owner = [=]{
                    target->m_game->remove_listeners(key);
                    target->m_game->queue_action([=]{
                        if (owner->alive()) {
                            if (player_ptr p = target_card->owner) {
                                p->disown_card(target_card);
                            }
                            target_card->add_short_pause();

                            if (pocket == pocket_type::player_table) {
                                owner->equip_card(target_card);
                                
                                if (num_cubes) {
                                    target_card->add_cubes(num_cubes);
                                }
                                if (tracked_player) {
                                    apply_pardner_token(target_card, owner, tracked_player);
                                }
                            } else {
                                owner->add_to_hand(target_card);
                            }
                        } else {
                            target_card->add_short_pause();
                            target->discard_card(target_card);
                        }
                    });
                };

                target->m_game->add_listener<event_type::on_turn_end>(key, [=](player_ptr origin, bool skipped) {
                    if (origin == target) {
                        return_to_owner();
                    }
                });

                target->m_game->add_listener<event_type::on_discard_hand_card>(key, [=](player_ptr origin, card_ptr e_origin_card, bool used) {
                    if (origin == target && e_origin_card == target_card && used) {
                        return_to_owner();
                    }
                });
            }
        });
    }

    void equip_ray_owe::on_disable(card_ptr origin_card, player_ptr target) {
        target->m_game->remove_listeners({ origin_card, 0 });
    }
}