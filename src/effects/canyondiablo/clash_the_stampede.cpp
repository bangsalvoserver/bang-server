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

    struct request_clash_the_stampede : request_base, interface_target_set_players {
        request_clash_the_stampede(card_ptr origin_card, player_ptr target)
            : request_base(origin_card, nullptr, target)
            , num_cards{get_max_num_cards(target)} {}
        
        int num_cards;

        bool in_target_set(const_player_ptr target_player) const override {
            return target_player != target && target_player->m_hand.size() == num_cards;
        }

        void on_update() override {
            auto target_set = origin->m_game->m_players | rv::filter(std::bind_front(&request_clash_the_stampede::in_target_set, this));
            if (num_cards == 0 || target_set.empty()) {
                target->m_game->pop_request();
            } else if (player_ptr target_player = get_single_element(target_set)) {
                on_pick_player(target_player);
            }
        }

        void on_pick_player(player_ptr target_player) {
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

    game_string effect_clash_the_stampede::get_error(card_ptr origin_card, player_ptr origin, player_ptr target) {
        if (origin->m_game->top_request<request_clash_the_stampede>(origin) == nullptr) {
            return "ERROR_INVALID_ACTION";
        }
        return {};
    }

    void effect_clash_the_stampede::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        auto req = origin->m_game->top_request<request_clash_the_stampede>();
        req->on_pick_player(target);
    }
}