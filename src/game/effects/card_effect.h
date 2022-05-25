#ifndef __CARD_EFFECT_H__
#define __CARD_EFFECT_H__

#include "../card_enums.h"
#include "../format_str.h"

#include <memory>

namespace banggame {

    struct player;
    struct card;

    template<target_type E> struct game_target_transform {
        using type = void;
    };
    template<> struct game_target_transform<target_type::player> {
        using type = player*;
    };
    template<> struct game_target_transform<target_type::card> {
        using type = card*;
    };
    template<> struct game_target_transform<target_type::cards_other_players> {
        using type = std::vector<card*>;
    };
    template<> struct game_target_transform<target_type::cube> {
        using type = std::vector<card*>;
    };

    using play_card_target = enums::enum_variant<target_type, game_target_transform>;

    template<target_type E> struct tagged_value {};

    template<target_type E>
    requires (!std::is_void_v<typename play_card_target::value_type<E>>)
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
        opt_fmt_str on_prompt(card *target_card, player *target);
    };

    struct request_base {
        request_base(card *origin_card, player *origin, player *target, effect_flags flags = {})
            : origin_card(origin_card), origin(origin), target(target), flags(flags) {}
        
        virtual ~request_base() {}

        card *origin_card;
        player *origin;
        player *target;
        effect_flags flags;

        virtual game_formatted_string status_text(player *owner) const = 0;

        virtual bool can_pick(pocket_type pocket, player *target, card *target_card) const {
            return false;
        }

        virtual void on_pick(pocket_type pocket, player *target, card *target_card);

        virtual void tick() {}
    };

    struct resolvable_request {
        virtual void on_resolve() = 0;
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

    struct timer_request : request_base, std::enable_shared_from_this<timer_request> {
        timer_request(card *origin_card, player *origin, player *target, effect_flags flags = {}, int duration = 200)
            : request_base(origin_card, origin, target, flags)
            , duration(duration) {}

        int duration;

        void tick() override final;
        virtual void on_finished();
    };

    struct selection_picker : request_base {
        using request_base::request_base;

        bool can_pick(pocket_type pocket, player *target_player, card *target_card) const override {
            return pocket == pocket_type::selection;
        }
    };

}


#endif