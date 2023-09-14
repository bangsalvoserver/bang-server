#ifndef __EVENT_CARD_KEY_H__
#define __EVENT_CARD_KEY_H__

#include "player.h"

namespace banggame {

    inline int get_card_order(const card *target_card) {
        return target_card ? target_card->order : 0;
    }

    struct event_card_key {
        card *target_card;
        int priority;

        event_card_key(card *target_card, int priority = 0)
            : target_card(target_card)
            , priority(priority) {}

        bool operator == (const event_card_key &other) const = default;

        auto operator <=> (const event_card_key &other) const {
            return target_card == other.target_card ?
                priority <=> other.priority :
                get_card_order(target_card) <=> get_card_order(other.target_card);
        }

        auto operator <=> (card *other) const {
            return get_card_order(target_card) <=> get_card_order(other);
        }

        struct priority_greater {
            bool operator()(const event_card_key &lhs, const event_card_key &rhs) const {
                return lhs.priority == rhs.priority ?
                    get_card_order(lhs.target_card) < get_card_order(rhs.target_card) :
                    lhs.priority > rhs.priority;
            }
        };
    };

}

#endif