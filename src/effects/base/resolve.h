#ifndef __BASE_RESOLVE_H__
#define __BASE_RESOLVE_H__

#include "cards/card_effect.h"

namespace banggame {

    enum class resolve_type {
        resolve,
        dismiss
    };

    struct effect_resolve {
        resolve_type type;
        effect_resolve(int type): type{static_cast<resolve_type>(type)} {}

        prompt_string on_prompt(card_ptr origin_card, player_ptr origin);
        bool can_play(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(resolve, effect_resolve)

    struct interface_resolvable {
        virtual void on_resolve() = 0;
        virtual resolve_type get_resolve_type() const { return resolve_type::resolve; }
        virtual prompt_string resolve_prompt() const { return {}; }
    };

    class request_resolvable: public request_base, public interface_resolvable {
    public:
        using request_base::request_base;

    protected:
        void auto_resolve();
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
}

#endif