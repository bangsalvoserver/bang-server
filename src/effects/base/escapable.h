#ifndef __BASE_ESCAPABLE_H__
#define __BASE_ESCAPABLE_H__

#include "cards/card_effect.h"

#include "resolve.h"

namespace banggame {

    enum class escape_type {
        no_escape,
        escape_timer,
        escape_no_timer
    };
    
    class escapable_request {
    public:
        virtual ~escapable_request() = default;
        
        size_t num_cards_used() const {
            return m_cards_used.size();
        }
        
        void add_card(card_ptr c) {
            m_cards_used.insert(c);
        }

        virtual bool can_escape(card_ptr c) const {
            return !m_cards_used.contains(c);
        }

        virtual prompt_string escape_prompt(player_ptr owner) const {
            return {};
        }

    private:
        std::set<card_ptr> m_cards_used;
    };

    struct request_escapable_resolvable : request_resolvable_timer, escapable_request {
        using request_resolvable_timer::request_resolvable_timer;
    
        escape_type get_escape_type() const;
    };

    struct request_targeting : request_escapable_resolvable {
        request_targeting(card_ptr origin_card, player_ptr origin, player_ptr target, card_ptr target_card, effect_flags flags = {}, int priority = 40)
            : request_escapable_resolvable(origin_card, origin, target, flags, priority)
            , target_card(target_card) {}
        
        card_ptr target_card;

        void on_update() override;

        virtual card_list get_highlights(player_ptr owner) const override;
    };

    namespace event_type {
        struct apply_escapable_modifier {
            card_ptr origin_card;
            player_ptr origin;
            const_player_ptr target;
            effect_flags flags;
            const escapable_request &req;
            nullable_ref<escape_type> value;
        };
    }

}

#endif