#ifndef __BASE_PICK_H__
#define __BASE_PICK_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_pick {
        bool can_play(card_ptr origin_card, player_ptr origin);
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target);
    };
    
    DEFINE_EFFECT(pick, effect_pick)

    struct effect_pick_player {
        bool can_play(card_ptr origin_card, player_ptr origin);
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target);
    };
    
    DEFINE_EFFECT(pick_player, effect_pick_player)

    struct interface_picking : interface_target_set_cards {
        bool in_target_set(const_card_ptr target_card) const final {
            return can_pick(target_card);
        }

        virtual bool can_pick(const_card_ptr target_card) const = 0;
        virtual void on_pick(card_ptr target_card) = 0;
        virtual prompt_string pick_prompt(card_ptr target_card) const { return {}; }
    };

    class request_picking : public request_base, public interface_picking {
    public:
        using request_base::request_base;

    protected:
        card_ptr get_auto_pick_target() const;
        void auto_pick();
    };

    class request_auto_pickable: public request_picking {
    public:
        using request_picking::request_picking;
    
    private:
        struct auto_pick_timer : request_timer {
            card_ptr target_card;

            auto_pick_timer(request_auto_pickable *request, card_ptr target_card);
            
            void on_finished() override {
                static_cast<request_auto_pickable *>(request)->on_pick(target_card);
            }
        };

        std::optional<auto_pick_timer> m_timer;

    public:
        request_timer *timer() override { return m_timer ? &*m_timer : nullptr; }
    
    protected:
        void auto_pick();
    };

    struct selection_picker : request_picking {
        using request_picking::request_picking;

        bool can_pick(const_card_ptr target_card) const override;
    };

    struct interface_picking_player : interface_target_set_players {
        bool in_target_set(const_player_ptr target_player) const final;

        virtual bool can_pick(const_player_ptr target_player) const = 0;
        virtual void on_pick(player_ptr target_player) = 0;
        virtual prompt_string pick_prompt(player_ptr target_player) const { return {}; }
    };

    class request_picking_player : public request_base, public interface_picking_player {
    public:
        using request_base::request_base;

    protected:
        player_ptr get_auto_pick_target() const;
        void auto_pick();
    };

    class request_auto_pickable_player: public request_picking_player {
    public:
        using request_picking_player::request_picking_player;
    
    private:
        struct auto_pick_timer : request_timer {
            player_ptr target_player;

            auto_pick_timer(request_auto_pickable_player *request, player_ptr target_player);
            
            void on_finished() override {
                static_cast<request_auto_pickable_player *>(request)->on_pick(target_player);
            }
        };

        std::optional<auto_pick_timer> m_timer;

    public:
        request_timer *timer() override { return m_timer ? &*m_timer : nullptr; }
    
    protected:
        void auto_pick();
    };

}

#endif