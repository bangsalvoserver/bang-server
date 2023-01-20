#ifndef __HOLDERS_H__
#define __HOLDERS_H__

#include <concepts>
#include <memory>

#include "utils/reflector.h"
#include "cards/card_effect.h"

namespace banggame {
    
    DEFINE_STRUCT(effect_holder,
        (target_type, target)
        (target_player_filter, player_filter)
        (target_card_filter, card_filter)
        (int8_t, effect_value)
        (int8_t, target_value)
        (effect_type, type),

        verify_result verify(card *origin_card, player *origin) const;
        game_string on_prompt(card *origin_card, player *origin) const;
        void on_play(card *origin_card, player *origin, effect_flags flags = {}) const;

        verify_result verify(card *origin_card, player *origin, player *target) const;
        game_string on_prompt(card *origin_card, player *origin, player *target) const;
        void on_play(card *origin_card, player *origin, player *target, effect_flags flags = {}) const;
        
        verify_result verify(card *origin_card, player *origin, card *target) const;
        game_string on_prompt(card *origin_card, player *origin, card *target) const;
        void on_play(card *origin_card, player *origin, card *target, effect_flags flags = {}) const;
    )
    
    DEFINE_STRUCT(equip_holder,
        (short, effect_value)
        (equip_type, type),

        game_string on_prompt(player *origin, card *target_card, player *target) const;
        void on_equip(card *target_card, player *target) const;
        void on_enable(card *target_card, player *target) const;
        void on_disable(card *target_card, player *target) const;
        void on_unequip(card *target_card, player *target) const;
    )

    DEFINE_STRUCT(modifier_holder,
        (card_modifier_type, type),
        
        game_string on_prompt(card *origin_card, player *origin, card *playing_card) const;
        verify_result verify(card *origin_card, player *origin, card *playing_card) const;
    )

    DEFINE_STRUCT(mth_holder,
        (mth_type, type),
        
        verify_result verify(card *origin_card, player *origin, const target_list &targets) const;
        game_string on_prompt(card *origin_card, player *origin, const target_list &targets) const;
        void on_play(card *origin_card, player *origin, const target_list &targets) const;
    )

    DEFINE_STRUCT(tag_holder,
        (short, tag_value)
        (tag_type, type)
    )

    class request_holder {
    public:
        request_holder() = default;
        
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
        bool is_sent() const {
            return m_value->sent;
        }
        bool is_popped() const {
            return m_value->popped;
        }
        void set_popped() {
            m_value->popped = true;
        }
        game_string status_text(player *owner) const {
            return m_value->status_text(owner);
        }

        bool can_pick(card *target_card) const {
            return m_value->can_pick(target_card);
        }

        void on_pick(card *target_card) {
            m_value->on_pick(target_card);
        }

        std::vector<card *> get_highlights() const {
            return m_value->get_highlights();
        }

        void on_update() {
            m_value->on_update();
        }

        void start(ticks total_update_time) {
            if (auto *t = m_value->timer()) {
                t->start(total_update_time);
            }
            m_value->sent = true;
        }

        void tick(request_queue *queue) {
            if (auto *t = m_value->timer()) {
                t->tick(queue);
            }
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