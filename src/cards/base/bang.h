#ifndef __BASE_BANG_H__
#define __BASE_BANG_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"
#include "prompts.h"

namespace banggame {
    
    struct effect_bang : prompt_target_ghost, bot_suggestion::target_enemy {
        void on_play(card *origin_card, player *origin, player *target, effect_flags flags = {});
    };

    struct effect_bangcard : prompt_target_ghost, bot_suggestion::target_enemy {
        void on_play(card *origin_card, player *origin, player *target);
    };

    struct handler_play_as_bang {
        bool on_check_target(card *origin_card, player *origin, const effect_context &ctx, card *chosen_card, const play_card_target &target);
        game_string on_prompt(card *origin_card, player *origin, const effect_context &ctx, card *chosen_card, const play_card_target &target);
        void on_play(card *origin_card, player *origin, const effect_context &ctx, card *chosen_card, const play_card_target &target);
    };

    struct effect_banglimit {
        game_string get_error(card *origin_card, player *origin, const effect_context &ctx);
        void on_play(card *origin_card, player *origin, const effect_context &ctx);
    };

    struct modifier_bangmod {
        bool valid_with_modifier(card *origin_card, player *origin, card *target_card);
        bool valid_with_card(card *origin_card, player *origin, card *target_card);
    };

    class missable_request {
    public:
        size_t num_cards_used() const {
            return m_cards_used.size();
        }
        
        void add_card(card *c) {
            m_cards_used.push_back(c);
        }

        virtual bool can_miss(card *c) const {
            return std::ranges::find(m_cards_used, c) == m_cards_used.end();
        }

        virtual void on_miss() = 0;

    private:
        std::vector<card *> m_cards_used;
    };

    struct request_bang : request_base, missable_request, resolvable_request {
        request_bang(card *origin_card, player *origin, player *target, effect_flags flags = {})
            : request_base(origin_card, origin, target, flags) {}

        ~request_bang();

        int bang_strength = 1;
        int bang_damage = 1;
        bool unavoidable = false;

        std::function<void()> cleanup_function;

        void on_cleanup(std::function<void()> &&fun);

        void on_update() override;

        bool can_miss(card *c) const override;

        void on_miss() override;
        void on_resolve() override;

        game_string status_text(player *owner) const override;
    };

}

#endif