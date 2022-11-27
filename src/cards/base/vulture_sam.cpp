#include "vulture_sam.h"

#include "game/game.h"

namespace banggame {
    
    inline bool is_non_black(card *target_card) {
        return target_card->color != card_color_type::black;
    }

    inline void steal_card(player *origin, player *target, card *target_card) {
        if (target_card->pocket == pocket_type::player_hand) {
            target->m_game->add_log(update_target::includes(origin, target), "LOG_STOLEN_CARD", origin, target, target_card);
            target->m_game->add_log(update_target::excludes(origin, target), "LOG_STOLEN_CARD_FROM_HAND", origin, target);
        } else {
            target->m_game->add_log("LOG_STOLEN_CARD", origin, target, target_card);
        }
        origin->steal_card(target_card);
    }

    inline bool is_vulture_sam(player *target) {
        return target->m_game->call_event<event_type::verify_card_taker>(target, equip_type::vulture_sam, false);
    }

    struct request_multi_vulture_sam : request_base {
        request_multi_vulture_sam(card *origin_card, player *origin, player *target, effect_flags flags = {})
            : request_base(origin_card, origin, target, flags | effect_flags::auto_pick) {}

        bool can_pick(card *target_card) const override {
            return (target_card->pocket == pocket_type::player_hand || target_card->pocket == pocket_type::player_table)
                && target_card->owner == origin
                && target_card->color != card_color_type::black;
        }

        void on_pick(card *target_card) override {
            auto lock = target->m_game->lock_updates(true);
            if (target_card->pocket == pocket_type::player_hand) {
                target_card = origin->random_hand_card();
            }
            steal_card(target, origin, target_card);

            if (!origin->only_black_cards_equipped()) {
                player_iterator next_target(target);
                do {
                    ++next_target;
                } while (next_target == origin || !is_vulture_sam(next_target));
                target->m_game->queue_request_front<request_multi_vulture_sam>(origin_card, origin, next_target);
            }
        }

        bool auto_resolve() override {
            if (request_base::auto_resolve()) {
                return true;
            } else if (std::ranges::all_of(origin->m_table, [](card *c) { return c->color == card_color_type::black; })) {
                on_pick(origin->m_hand.front());
                return true;
            } else {
                return false;
            }
        }

        game_string status_text(player *owner) const override {
            if (owner == target) {
                return {"STATUS_MULTI_VULTURE_SAM", origin_card, origin};
            } else {
                return {"STATUS_MULTI_VULTURE_SAM_OTHER", origin_card, target, origin};
            }
        }
    };
    
    void equip_vulture_sam::on_enable(card *target_card, player *origin) {
        origin->m_game->add_listener<event_type::verify_card_taker>(target_card, [=](player *e_target, equip_type type, bool &value){
            if (type == equip_type::vulture_sam && e_target == origin) {
                value = true;
            }
        });
        origin->m_game->add_listener<event_type::on_player_death>(target_card, [=](player *killer, player *target) {
            if (target->only_black_cards_equipped()) return;

            std::vector<player *> range_targets;
            int count = target->m_game->num_alive();
            player_iterator it{target};
            do {
                ++it;
                if (target->m_game->call_event<event_type::verify_card_taker>(it, equip_type::vulture_sam, false)) {
                    range_targets.push_back(it);
                }
            } while (--count != 0);
            if (range_targets.size() == 1) {
                while (auto non_black_cards = target->m_table | std::views::filter(is_non_black)) {
                    steal_card(origin, target, non_black_cards.front());
                }
                while (!target->m_hand.empty()) {
                    steal_card(origin, target, target->m_hand.front());
                }
            } else if (!range_targets.empty() && range_targets.front() == origin) {
                origin->m_game->queue_request_front<request_multi_vulture_sam>(target_card, target, origin, effect_flags::auto_pick);
            }
        });
    }

}