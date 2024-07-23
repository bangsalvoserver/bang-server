#ifndef __BASE_PICK_H__
#define __BASE_PICK_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_pick {
        game_string get_error(card *origin_card, player *origin, card *target);
        game_string on_prompt(card *origin_card, player *origin, card *target);
        void on_play(card *origin_card, player *origin, card *target);
    };
    
    DEFINE_EFFECT(pick, effect_pick)

    struct interface_picking : interface_target_set_cards {
        bool in_target_set(const card *target_card) const final {
            return can_pick(target_card);
        }

        virtual bool can_pick(const card *target_card) const = 0;
        virtual void on_pick(card *target_card) = 0;
        virtual game_string pick_prompt(card *target_card) const { return {}; }
    };

    class request_picking : public request_base, public interface_picking {
    public:
        using request_base::request_base;

    protected:
        void auto_pick();
    };

    struct selection_picker : request_picking {
        using request_picking::request_picking;

        bool can_pick(const card *target_card) const override;
    };

}

#endif