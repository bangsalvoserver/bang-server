#ifndef __BASE_EQUIPS_H__
#define __BASE_EQUIPS_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_mustang {
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };

    struct effect_scope {
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };

    struct effect_predraw_check {
        int priority;
        effect_predraw_check(int priority) : priority(priority) {}

        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };

    struct effect_jail : event_based_effect {
        void on_enable(card *target_card, player *target);
    };

    struct effect_dynamite : event_based_effect {
        void on_enable(card *target_card, player *target);
    };

    struct effect_horse {
        opt_fmt_str on_prompt(card *target_card, player *target);
        void on_equip(card *target_card, player *target);
    };

    struct effect_weapon_base {
        int range;
        effect_weapon_base(int value) : range(value) {}

        void on_equip(card *target_card, player *target);
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };

    struct effect_weapon : effect_weapon_base {
        effect_weapon(int value) : effect_weapon_base(value) {}
        opt_fmt_str on_prompt(card *target_card, player *target);
    };

    struct effect_guitar : effect_weapon_base {
        effect_guitar() : effect_weapon_base(0) {}
    };

    struct effect_volcanic : event_based_effect {
        void on_enable(card *target_card, player *target);
    };

    struct effect_boots : event_based_effect {
        void on_enable(card *target_card, player *target);
    };

    struct effect_horseshoe {
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };
}

#endif