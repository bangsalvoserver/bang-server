#include "clash_the_stampede.h"

#include "effects/wildwestshow/youl_grinner.h"

#include "game/game.h"
#include "game/possible_to_play.h"

namespace banggame {

    static int get_max_num_cards(player_ptr target) {
        return rn::max(target->m_game->m_players
            | rv::transform([](player_ptr p) {
                return static_cast<int>(p->m_hand.size());
            }));
    }

    struct request_clash_the_stampede : request_picking_player {
        request_clash_the_stampede(card_ptr origin_card, player_ptr target)
            : request_picking_player(origin_card, nullptr, target)
            , num_cards{get_max_num_cards(target)} {}
        
        int num_cards;

        bool can_pick(const_player_ptr target_player) const override {
            return target_player != target && target_player->m_hand.size() == num_cards;
        }

        void on_update() override {
            if (num_cards == 0 || rn::none_of(target->m_game->m_players, [&](const_player_ptr p) { return can_pick(p); })) {
                target->m_game->pop_request();
            } else {
                auto_pick();
            }
        }

        void on_pick(player_ptr target_player) override {
            target->m_game->pop_request();
            target->m_game->queue_request<request_youl_grinner>(origin_card, target, target_player);
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_CLASH_THE_STAMPEDE", origin_card};
            } else {
                return {"STATUS_CLASH_THE_STAMPEDE_OTHER", target, origin_card};
            }
        }
    };

    void equip_clash_the_stampede::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_start>(target_card, [=](player_ptr origin) {
            if (target == origin) {
                target->m_game->queue_request<request_clash_the_stampede>(target_card, target);
            }
        });
    }

}