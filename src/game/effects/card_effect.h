#ifndef __CARD_EFFECT_H__
#define __CARD_EFFECT_H__

#include "../card_enums.h"
#include "../format_str.h"

#include "utils/nullable.h"

#include <memory>

namespace banggame {

    struct player;
    struct card;

    DEFINE_ENUM_VARIANT(play_card_target, target_type,
        (player,                player *)
        (conditional_player,    nullable<player>)
        (card,                  card *)
        (extra_card,            nullable<card>)
        (cards,                 std::vector<card *>)
        (cards_other_players,   std::vector<card *>)
        (select_cubes,          std::vector<card *>)
        (self_cubes,            int)
    )

    template<target_type E> struct tagged_value {};

    template<target_type E>
    requires (play_card_target::has_type<E>)
    struct tagged_value<E> {
        typename play_card_target::value_type<E> value;
    };

    template<target_type E>
    using opt_tagged_value = std::optional<tagged_value<E>>;

    using target_list = std::vector<play_card_target>;

    struct effect_empty {
        void on_play(card *origin_card, player *origin) {}
        void on_play(card *origin_card, player *origin, player *target) {}
        void on_play(card *origin_card, player *origin, card *target) {}
    };

    struct event_based_effect {
        void on_disable(card *target_card, player *target);
    };

    struct effect_prompt_on_self_equip {
        game_string on_prompt(player *origin, card *target_card, player *target);
    };

    struct request_base {
        request_base(card *origin_card, player *origin, player *target, effect_flags flags = {})
            : origin_card(origin_card), origin(origin), target(target), flags(flags) {}
        
        virtual ~request_base() {}

        card *origin_card;
        player *origin;
        player *target;
        effect_flags flags;

        virtual void tick() {}

        virtual void add_pending_confirm(player *p) {}
        virtual void confirm_player(player *p) {}

        virtual game_string status_text(player *owner) const = 0;

        virtual bool can_pick(pocket_type pocket, player *target, card *target_card) const {
            return false;
        }

        virtual void on_pick(pocket_type pocket, player *target, card *target_card);

        virtual bool can_respond(player *target, card *target_card) const;
    };

    struct timer_request : request_base, std::enable_shared_from_this<timer_request> {
        static constexpr int default_duration = 300;

        timer_request(card *origin_card, player *origin, player *target, effect_flags flags = {}
            , int duration = default_duration)
            : request_base(origin_card, origin, target, flags)
            , duration(duration) {}

        int duration;
        
        std::vector<player *> awaiting_confirms;
        int auto_confirm_timer = 600;

        void add_pending_confirm(player *p) override final;
        void confirm_player(player *p) override final;

        void tick() override final;
        virtual void on_finished() {}
    };

    class cleanup_request {
    public:
        cleanup_request() = default;
        ~cleanup_request() {
            if (m_fun) {
                m_fun();
                m_fun = nullptr;
            }
        }

        cleanup_request(const cleanup_request &) = delete;
        cleanup_request(cleanup_request &&other) noexcept
            : m_fun(std::move(other.m_fun))
        {
            other.m_fun = nullptr;
        }

        cleanup_request &operator = (const cleanup_request &) = delete;
        cleanup_request &operator = (cleanup_request &&other) noexcept {
            m_fun = std::move(other.m_fun);
            other.m_fun = nullptr;
            return *this;
        }

        void on_cleanup(std::function<void()> &&fun) {
            m_fun = std::move(fun);
        }

    private:
        std::function<void()> m_fun;
    };

    struct selection_picker : request_base {
        using request_base::request_base;

        bool can_pick(pocket_type pocket, player *target_player, card *target_card) const override {
            return pocket == pocket_type::selection;
        }
    };

    struct resolvable_request {
        virtual void on_resolve() = 0;
    };

}


#endif