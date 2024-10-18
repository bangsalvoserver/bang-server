#ifndef __BASE_BANG_H__
#define __BASE_BANG_H__

#include "cards/card_effect.h"
#include "resolve.h"

namespace banggame {
    
    struct effect_bang {
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags = {});
    };

    DEFINE_EFFECT(bang, effect_bang)

    struct effect_bangcard {
        game_string get_error(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx);
        game_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target);
    };

    DEFINE_EFFECT(bangcard, effect_bangcard)

    struct handler_play_as_bang {
        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_context &ctx, card_ptr chosen_card, player_ptr target);
        game_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_context &ctx, card_ptr chosen_card, player_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx, card_ptr chosen_card, player_ptr target);
    };

    DEFINE_MTH(play_as_bang, handler_play_as_bang)

    struct effect_banglimit {
        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_context &ctx);
        void on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx);
    };

    DEFINE_EFFECT(banglimit, effect_banglimit)

    struct equip_treat_as_bang {
        player_flag flag;
        equip_treat_as_bang(int value);
        
        void on_enable(card_ptr target_card, player_ptr target);
        void on_disable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(treat_as_bang, equip_treat_as_bang)

    struct modifier_bangmod {
        bool valid_with_equip(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
            return false;
        }
        bool valid_with_modifier(card_ptr origin_card, player_ptr origin, card_ptr target_card);
        bool valid_with_card(card_ptr origin_card, player_ptr origin, card_ptr target_card);
    };

    DEFINE_MODIFIER(bangmod, modifier_bangmod)

    class missable_request {
    public:
        size_t num_cards_used() const {
            return m_cards_used.size();
        }
        
        void add_card(card_ptr c) {
            m_cards_used.push_back(c);
        }

        virtual bool can_miss(card_ptr c) const {
            return rn::find(m_cards_used, c) == m_cards_used.end();
        }

        virtual void on_miss(card_ptr c, effect_flags missed_flags = {}) = 0;

    private:
        card_list m_cards_used;
    };

    struct request_bang : request_resolvable, missable_request {
        using request_resolvable::request_resolvable;

        int bang_strength = 1;
        int bang_damage = 1;
        bool unavoidable = false;

        void on_update() override;

        bool can_miss(card_ptr c) const override;

        void on_miss(card_ptr c, effect_flags missed_flags = {}) override;
        void on_resolve() override;

        game_string status_text(player_ptr owner) const override;
    };

    using shared_request_bang = std::shared_ptr<request_bang>;
    
    namespace event_type {
        struct count_bangs_played {
            const_player_ptr origin;
            nullable_ref<int> num_bangs_played;
        };

        struct apply_bang_modifier {
            player_ptr origin;
            shared_request_bang req;
        };
        
        struct check_bang_target {
            card_ptr origin_card;
            player_ptr origin;
            player_ptr target;
            effect_flags flags;
            nullable_ref<game_string> out_error;
        };
        
        struct on_missed {
            card_ptr origin_card;
            player_ptr origin;
            player_ptr target;
            card_ptr missed_card;
            effect_flags flags;
        };
    }

    class respondable_with_bang {
    public:
        virtual void respond_with_bang() = 0;
    };

    struct effect_bangresponse {
        bool can_play(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };
    
    DEFINE_EFFECT(bangresponse, effect_bangresponse)

}

#endif