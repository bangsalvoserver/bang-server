#ifndef __BASE_BANG_H__
#define __BASE_BANG_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"
#include "resolve.h"
#include "prompts.h"

namespace banggame {
    
    struct effect_bang : prompt_target_ghost, bot_suggestion::target_enemy {
        void on_play(card *origin_card, player *origin, player *target, effect_flags flags = {});
    };

    DEFINE_EFFECT(bang, effect_bang)

    struct effect_bangcard : prompt_target_ghost, bot_suggestion::target_enemy {
        game_string get_error(card *origin_card, player *origin, player *target, const effect_context &ctx);
        void on_play(card *origin_card, player *origin, player *target);
    };

    DEFINE_EFFECT(bangcard, effect_bangcard)

    struct handler_play_as_bang {
        bool on_check_target(card *origin_card, player *origin, const effect_context &ctx, card *chosen_card, player *target);
        game_string get_error(card *origin_card, player *origin, const effect_context &ctx, card *chosen_card, player *target);
        game_string on_prompt(card *origin_card, player *origin, const effect_context &ctx, card *chosen_card, player *target);
        void on_play(card *origin_card, player *origin, const effect_context &ctx, card *chosen_card, player *target);
    };

    DEFINE_MTH(play_as_bang, handler_play_as_bang)

    struct effect_banglimit {
        game_string get_error(card *origin_card, player *origin, const effect_context &ctx);
        void on_play(card *origin_card, player *origin, const effect_context &ctx);
    };

    DEFINE_EFFECT(banglimit, effect_banglimit)

    struct equip_treat_as_bang {
        player_flags flag;
        equip_treat_as_bang(int value);
        
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };

    DEFINE_EQUIP(treat_as_bang, equip_treat_as_bang)

    struct modifier_bangmod {
        bool valid_with_equip(card *origin_card, player *origin, card *target_card) {
            return false;
        }
        bool valid_with_modifier(card *origin_card, player *origin, card *target_card);
        bool valid_with_card(card *origin_card, player *origin, card *target_card);
    };

    DEFINE_MODIFIER(bangmod, modifier_bangmod)

    class missable_request {
    public:
        size_t num_cards_used() const {
            return m_cards_used.size();
        }
        
        void add_card(card *c) {
            m_cards_used.push_back(c);
        }

        virtual bool can_miss(card *c) const {
            return rn::find(m_cards_used, c) == m_cards_used.end();
        }

        virtual void on_miss(card *c, effect_flags missed_flags = {}) = 0;

    private:
        std::vector<card *> m_cards_used;
    };

    struct request_bang : request_resolvable, missable_request {
        using request_resolvable::request_resolvable;

        int bang_strength = 1;
        int bang_damage = 1;
        bool unavoidable = false;

        void on_update() override;

        bool can_miss(card *c) const override;

        void on_miss(card *c, effect_flags missed_flags = {}) override;
        void on_resolve() override;

        game_string status_text(player *owner) const override;
    };

    using shared_request_bang = std::shared_ptr<request_bang>;
    
    namespace event_type {
        struct count_bangs_played {
            player *origin;
            nullable_ref<int> num_bangs_played;
        };

        struct apply_bang_modifier {
            player *origin;
            shared_request_bang req;
        };
        
        struct check_bang_target {
            card *origin_card;
            player *origin;
            player *target;
            effect_flags flags;
            nullable_ref<game_string> out_error;
        };
        
        struct on_missed {
            card *origin_card;
            player *origin;
            player *target;
            card *missed_card;
            effect_flags flags;
        };
    }

    class respondable_with_bang {
    public:
        virtual void respond_with_bang() = 0;
    };

    struct effect_bangresponse {
        bool can_play(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };
    
    DEFINE_EFFECT(bangresponse, effect_bangresponse)

}

#endif