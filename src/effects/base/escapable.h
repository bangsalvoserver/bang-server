#ifndef __BASE_ESCAPABLE_H__
#define __BASE_ESCAPABLE_H__

#include "cards/card_effect.h"

namespace banggame {

    enum class escape_type {
        no_escape,
        escape_timer,
        escape_no_timer
    };

    namespace event_type {
        struct apply_escapable_modifier {
            card_ptr origin_card;
            player_ptr origin;
            const_player_ptr target;
            effect_flags flags;
            nullable_ref<escape_type> value;
        };
    }

    escape_type get_escape_type(player_ptr origin, player_ptr target, card_ptr origin_card, effect_flags flags);
    
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

}

#endif