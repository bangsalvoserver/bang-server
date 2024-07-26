#include "vulture_sam.h"

#include "deathsave.h"
#include "pick.h"

#include "game/game.h"

namespace banggame {

    inline void steal_card(player_ptr origin, player_ptr target, card_ptr target_card) {
        if (target_card->visibility != card_visibility::shown) {
            target->m_game->add_log(update_target::includes(origin, target), "LOG_STOLEN_CARD", origin, target, target_card);
            target->m_game->add_log(update_target::excludes(origin, target), "LOG_STOLEN_CARD_FROM_HAND", origin, target);
        } else {
            target->m_game->add_log("LOG_STOLEN_CARD", origin, target, target_card);
        }
        origin->steal_card(target_card);
    }

    static card_ptr get_vulture_sam(player_ptr target) {
        card_ptr origin_card = nullptr;
        target->m_game->call_event(event_type::check_card_taker{ target, card_taker_type::dead_players, origin_card });
        return origin_card;
    }

    struct request_multi_vulture_sam : request_picking {
        request_multi_vulture_sam(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags = {})
            : request_picking(origin_card, origin, target, flags, 200) {}

        bool can_pick(const_card_ptr target_card) const override {
            return (target_card->pocket == pocket_type::player_hand || target_card->pocket == pocket_type::player_table)
                && target_card->owner == origin
                && !target_card->is_black();
        }

        void on_pick(card_ptr target_card) override {
            target->m_game->pop_request();
            if (target_card->pocket == pocket_type::player_hand) {
                target_card = origin->random_hand_card();
            }
            steal_card(target, origin, target_card);

            if (!origin->empty_hand() || !origin->empty_table()) {
                for (player_ptr next_target : range_other_players(target)) {
                    if (next_target == origin) continue;

                    if (card_ptr next_origin_card = get_vulture_sam(next_target)) {
                        target->m_game->queue_request<request_multi_vulture_sam>(next_origin_card, origin, next_target);
                        break;
                    }
                }
            }
        }

        void on_update() override {
            auto cards = rv::concat(
                origin->m_table | rv::remove_if(&card::is_black),
                origin->m_hand | rv::take(1)
            );
            if (rn::distance(cards) == 1) {
                on_pick(cards.front());
            }
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_MULTI_VULTURE_SAM", origin_card, origin};
            } else {
                return {"STATUS_MULTI_VULTURE_SAM_OTHER", origin_card, target, origin};
            }
        }
    };
    
    void equip_vulture_sam::on_enable(card_ptr target_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::check_card_taker>(target_card, [=](player_ptr e_target, card_taker_type type, card* &value){
            if (type == card_taker_type::dead_players && e_target == origin) {
                value = target_card;
            }
        });
        origin->m_game->add_listener<event_type::on_player_death>(target_card, [=](player_ptr killer, player_ptr target) {
            if (target->empty_hand() && target->empty_table()) return;
            
            for (card_ptr target_card : target->m_table) {
                target_card->set_inactive(false);
            }

            player_list range_targets;
            for (player_ptr p : range_other_players(target)) {
                if (get_vulture_sam(p)) {
                    range_targets.push_back(p);
                }
            }
            if (range_targets.size() == 1) {
                for (card_ptr target_card : target->m_table
                    | rv::remove_if(&card::is_black)
                    | rn::to_vector
                ) {
                    steal_card(origin, target, target_card);
                }
                while (!target->empty_hand()) {
                    steal_card(origin, target, target->m_hand.front());
                }
            } else if (!range_targets.empty() && range_targets.front() == origin) {
                origin->m_game->queue_request<request_multi_vulture_sam>(target_card, target, origin);
            }
        });
    }

}