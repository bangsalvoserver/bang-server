#include "vulture_sam.h"

#include "game/game.h"

namespace banggame {

    inline void steal_card(player *origin, player *target, card *target_card) {
        if (target_card->visibility != card_visibility::shown) {
            target->m_game->add_log(update_target::includes(origin, target), "LOG_STOLEN_CARD", origin, target, target_card);
            target->m_game->add_log(update_target::excludes(origin, target), "LOG_STOLEN_CARD_FROM_HAND", origin, target);
        } else {
            target->m_game->add_log("LOG_STOLEN_CARD", origin, target, target_card);
        }
        origin->steal_card(target_card);
    }

    static card *get_vulture_sam(player *target) {
        return target->m_game->call_event<event_type::check_card_taker>(target, equip_type::vulture_sam, nullptr);
    }

    struct request_multi_vulture_sam : request_base {
        request_multi_vulture_sam(card *origin_card, player *origin, player *target, effect_flags flags = {})
            : request_base(origin_card, origin, target, flags) {}

        bool can_pick(card *target_card) const override {
            return (target_card->pocket == pocket_type::player_hand || target_card->pocket == pocket_type::player_table)
                && target_card->owner == origin
                && !target_card->is_black();
        }

        void on_pick(card *target_card) override {
            target->m_game->invoke_action([&]{
                target->m_game->pop_request();
                if (target_card->pocket == pocket_type::player_hand) {
                    target_card = origin->random_hand_card();
                }
                steal_card(target, origin, target_card);

                if (!origin->only_black_cards_equipped()) {
                    player_iterator next_target(target);
                    card *next_origin_card = nullptr;
                    do {
                        ++next_target;
                        next_origin_card = get_vulture_sam(*next_target);
                    } while (*next_target == origin || !next_origin_card);
                    target->m_game->queue_request_front<request_multi_vulture_sam>(next_origin_card, origin, *next_target);
                }
            });
        }

        void on_update() override {
            auto cards = ranges::views::concat(
                origin->m_table | ranges::views::remove_if(&card::is_black),
                origin->m_hand | ranges::views::take(1)
            );
            if (std::ranges::distance(cards) == 1) {
                on_pick(cards.front());
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
        origin->m_game->add_listener<event_type::check_card_taker>(target_card, [=](player *e_target, equip_type type, card* &value){
            if (type == equip_type::vulture_sam && e_target == origin) {
                value = target_card;
            }
        });
        origin->m_game->add_listener<event_type::on_player_death>(target_card, [=](player *killer, player *target) {
            if (target->only_black_cards_equipped()) return;
            target->untap_inactive_cards();

            std::vector<player *> range_targets;
            int count = target->m_game->num_alive();
            player_iterator it{target};
            do {
                ++it;
                if (get_vulture_sam(*it)) {
                    range_targets.push_back(*it);
                }
            } while (--count != 0);
            if (range_targets.size() == 1) {
                for (card *target_card : target->m_table
                    | ranges::views::remove_if(&card::is_black)
                    | ranges::to<std::vector>
                ) {
                    steal_card(origin, target, target_card);
                }
                while (!target->empty_hand()) {
                    steal_card(origin, target, target->m_hand.front());
                }
            } else if (!range_targets.empty() && range_targets.front() == origin) {
                origin->m_game->queue_request_front<request_multi_vulture_sam>(target_card, target, origin);
            }
        });
    }

}