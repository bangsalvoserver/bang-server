#ifndef __HOLDERS_H__
#define __HOLDERS_H__

#include <concepts>
#include <memory>

#include "utils/reflector.h"
#include "effects/card_effect.h"
#include "game_action.h"

namespace banggame {

    struct effect_holder {
        REFLECTABLE(
            (target_type) target,
            (target_player_filter) player_filter,
            (target_card_filter) card_filter,
            (short) effect_value,
            (effect_type) type
        )

        opt_game_str verify(card *origin_card, player *origin) const;
        opt_game_str on_prompt(card *origin_card, player *origin) const;
        void on_play(card *origin_card, player *origin, effect_flags flags) const;
        
        opt_game_str verify(card *origin_card, player *origin, player *target) const;
        opt_game_str on_prompt(card *origin_card, player *origin, player *target) const;
        void on_play(card *origin_card, player *origin, player *target, effect_flags flags) const;
        
        opt_game_str verify(card *origin_card, player *origin, card *target) const;
        opt_game_str on_prompt(card *origin_card, player *origin, card *target) const;
        void on_play(card *origin_card, player *origin, card *target, effect_flags flags) const;
    };
    
    struct equip_holder {
        REFLECTABLE(
            (short) effect_value,
            (equip_type) type
        )

        opt_game_str on_prompt(player *origin, card *target_card, player *target) const;
        void on_equip(card *target_card, player *target) const;
        void on_enable(card *target_card, player *target) const;
        void on_disable(card *target_card, player *target) const;
        void on_unequip(card *target_card, player *target) const;
    };

    struct mth_holder {
        REFLECTABLE((mth_type) type)
        
        opt_game_str verify(card *origin_card, player *origin, const target_list &targets) const;
        opt_game_str on_prompt(card *origin_card, player *origin, const target_list &targets) const;
        void on_play(card *origin_card, player *origin, const target_list &targets) const;
    };

    struct tag_holder {REFLECTABLE(
        (short) tag_value,
        (tag_type) type
    )};

    class request_holder {
    public:
        request_holder(std::shared_ptr<request_base> &&value)
            : m_value(std::move(value)) {}

        card *origin_card() const {
            return m_value->origin_card;
        }
        player *origin() const {
            return m_value->origin;
        }
        player *target() const {
            return m_value->target;
        }
        effect_flags flags() const {
            return m_value->flags;
        }
        game_string status_text(player *owner) const {
            return m_value->status_text(owner);
        }
        void add_pending_confirm(player *p) {
            m_value->add_pending_confirm(p);
        }
        void confirm_player(player *p) {
            m_value->confirm_player(p);
        }

        bool can_pick(pocket_type pocket, player *target, card *target_card) const {
            return m_value->can_pick(pocket, target, target_card);
        }

        void on_pick(pocket_type pocket, player *target, card *target_card) {
            auto copy = m_value;
            copy->on_pick(pocket, target, target_card);
        }

        bool can_respond(player *target, card *target_card) const {
            return m_value->can_respond(target, target_card);
        }

        void tick() {
            m_value->tick();
        }

        template<typename T> auto &get() {
            return dynamic_cast<T &>(*m_value);
        }

        template<typename T> const auto &get() const {
            return dynamic_cast<const T &>(*m_value);
        }

        template<typename T> auto *get_if() {
            return dynamic_cast<T *>(m_value.get());
        }

        template<typename T> const auto *get_if() const {
            return dynamic_cast<const T *>(m_value.get());
        }

        template<typename T> bool is() const {
            return get_if<T>() != nullptr;
        }

    private:
        std::shared_ptr<request_base> m_value;
    };
}

#endif