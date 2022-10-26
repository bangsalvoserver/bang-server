#include "vulture_sam.h"

#include "../../game.h"

namespace banggame {

    static bool is_vulture_sam(player *p) {
        return p->m_game->call_event<event_type::verify_card_taker>(p, equip_type::vulture_sam, false);
    }

    struct request_multi_vulture_sam : request_base {
        request_multi_vulture_sam(card *origin_card, player *origin, player *target, effect_flags flags = {})
            : request_base(origin_card, origin, target, flags | effect_flags::auto_pick) {}

        bool can_pick(pocket_type pocket, player *target_player, card *target_card) const override {
            return (pocket == pocket_type::player_hand || pocket == pocket_type::player_table)
                && target_player == origin
                && target_card->color != card_color_type::black;
        }

        void on_pick(pocket_type pocket, player *target_player, card *target_card) override {
            target->m_game->pop_request();

            if (pocket == pocket_type::player_hand) {
                target_card = origin->random_hand_card();
                target->m_game->add_log(update_target::includes(origin, target), "LOG_STOLEN_CARD", target, origin, target_card);
                target->m_game->add_log(update_target::excludes(origin, target), "LOG_STOLEN_CARD_FROM_HAND", target, origin);
            } else {
                target->m_game->add_log("LOG_STOLEN_CARD", target, origin, target_card);
            }
            target->steal_card(target_card);

            if (origin->only_black_cards_equipped()) {
                target->m_game->update_request();
            } else {
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
                on_pick(pocket_type::player_hand, origin, origin->m_hand.front());
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
    
    void effect_vulture_sam::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::verify_card_taker>(target_card, [=](player *e_target, equip_type type, bool &value){
            if (type == equip_type::vulture_sam && e_target == p) {
                value = true;
            }
        });
        p->m_game->add_listener<event_type::on_player_death>(target_card, [=](player *origin, player *target) {
            std::vector<player *> range_targets;
            int count = origin->m_game->num_alive();
            player_iterator it{target};
            do {
                ++it;
                if (origin->m_game->call_event<event_type::verify_card_taker>(it, equip_type::vulture_sam, false)) {
                    range_targets.push_back(it);
                }
            } while (--count != 0);
            if (range_targets.size() == 1) {
                std::vector<card *> target_cards;
                for (card *c : target->m_table) {
                    if (c->color != card_color_type::black) {
                        target_cards.push_back(c);
                    }
                }
                for (card *c : target->m_hand) {
                    target_cards.push_back(c);
                }

                for (card *c : target_cards) {
                    if (c->pocket == pocket_type::player_hand) {
                        target->m_game->add_log(update_target::includes(target, p), "LOG_STOLEN_CARD", p, target, c);
                        target->m_game->add_log(update_target::excludes(target, p), "LOG_STOLEN_CARD_FROM_HAND", p, target);
                    } else {
                        target->m_game->add_log("LOG_STOLEN_CARD", p, target, c);
                    }
                    p->steal_card(c);
                }
            } else if (!range_targets.empty() && range_targets.front() == p) {
                p->m_game->queue_request_front<request_multi_vulture_sam>(target_card, target, p, effect_flags::auto_pick);
            }
        });
    }

}