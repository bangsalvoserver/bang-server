#ifndef __EVENT_CARD_KEY_H__
#define __EVENT_CARD_KEY_H__

#include "card.h"

namespace banggame {

    inline int get_card_order(const_card_ptr target_card) {
        return target_card ? target_card->order : 0;
    }

    struct event_card_key {
        card_ptr target_card;
        int priority;

        event_card_key(card_ptr target_card, int priority = 0)
            : target_card(target_card)
            , priority(priority) {}

        bool operator == (const event_card_key &other) const = default;

        auto operator <=> (const event_card_key &other) const {
            if (target_card == other.target_card) {
                return priority <=> other.priority;
            } else {
                return get_card_order(target_card) <=> get_card_order(other.target_card);
            }
        }

        auto operator <=> (card_ptr other) const {
            return get_card_order(target_card) <=> get_card_order(other);
        }
    };

}

namespace std {
    template<> struct formatter<banggame::card_ptr> {
        constexpr auto parse(std::format_parse_context &ctx) {
            return ctx.begin();
        }

        auto format(banggame::card_ptr target_card, std::format_context &ctx) const {
            return std::format_to(ctx.out(), "{}", target_card
                ? std::string_view(target_card->name)
                : std::string_view("(unknown card)")
            );
        }
    };

    template<> struct formatter<banggame::event_card_key> {
        constexpr auto parse(std::format_parse_context &ctx) {
            return ctx.begin();
        }

        auto format(const banggame::event_card_key &key, std::format_context &ctx) const {
            return std::format_to(ctx.out(), "{: >5} - {}", key.priority, key.target_card);
        }
    };
}

#endif