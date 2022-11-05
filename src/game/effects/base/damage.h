#ifndef __BASE_DAMAGE_H__
#define __BASE_DAMAGE_H__

#include "../card_effect.h"

namespace banggame {
    
    struct effect_damage {
        int damage;
        effect_damage(int damage) : damage(std::max(1, damage)) {}

        game_string verify(card *origin_card, player *origin, effect_flags flags = {}) {
            return verify(origin_card, origin, origin, flags);
        }
        void on_play(card *origin_card, player *origin, effect_flags flags = {}) {
            on_play(origin_card, origin, origin, flags);
        }

        game_string verify(card *origin_card, player *origin, player *target, effect_flags flags = {});
        void on_play(card *origin_card, player *origin, player *target, effect_flags flags = {});
        void on_resolve(card *origin_card, player *origin, player *target, effect_flags flags = {});
    };

    struct timer_damaging : timer_request, cleanup_request {
        timer_damaging(card *origin_card, player *origin, player *target, int damage, effect_flags flags = {})
            : timer_request(origin_card, origin, target, flags & (effect_flags::is_bang | effect_flags::play_as_bang))
            , damage(damage) {}
        
        int damage;

        std::vector<card *> get_highlights() const override;
        void on_finished() override;
        game_string status_text(player *owner) const override;
    };
}

#endif