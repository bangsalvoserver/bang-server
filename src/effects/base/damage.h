#ifndef __BASE_DAMAGE_H__
#define __BASE_DAMAGE_H__

#include "cards/card_effect.h"

namespace banggame {

    namespace event_type {
        struct check_damage_response {
            player_ptr target;
            nullable_ref<bool> value;
        };
        
        struct on_hit {
            card_ptr origin_card;
            player_ptr origin;
            player_ptr target;
            int damage;
            effect_flags flags;
        };
    }
    
    struct effect_damage {
        int damage;
        effect_damage(int damage = 1) : damage{damage} {}

        game_string get_error(card_ptr origin_card, player_ptr origin, effect_flags flags = {});

        game_string on_prompt(card_ptr origin_card, player_ptr origin);
        
        void on_play(card_ptr origin_card, player_ptr origin, effect_flags flags = {}) {
            on_play(origin_card, origin, origin, flags);
        }

        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags = {});
    };

    DEFINE_EFFECT(damage, effect_damage)

    struct request_damage : request_base {
        request_damage(card_ptr origin_card, player_ptr origin, player_ptr target, int damage, effect_flags flags = {});

        int damage;

        player_ptr savior = nullptr;
        
        struct timer_damage : request_timer {
            explicit timer_damage(request_damage *request);
            
            void on_finished() override {
                static_cast<request_damage *>(request)->on_finished();
            }
        };

        timer_damage m_timer{this};
        request_timer *timer() override { return &m_timer; }

        card_list get_highlights(player_ptr owner) const override;
        void on_update() override;
        void on_finished();
        game_string status_text(player_ptr owner) const override;
    };
}

#endif