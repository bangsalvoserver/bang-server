#include "most_wanted.h"

#include "game/game.h"
#include "effects/base/resolve.h"
#include "effects/base/draw_check.h"

namespace banggame {

    static void resolve_most_wanted(card *origin_card, player *origin, player *target) {
        target->m_game->queue_request<request_check>(target, origin_card, std::not_fn(&card_sign::is_spades), [=](bool result) {
            if (!result) {
                target->damage(origin_card, origin, 1);
            }
        });
    }

    struct request_most_wanted : request_picking {
        request_most_wanted(card *origin_card, player *origin, player *target, effect_flags flags = {})
            : request_picking(origin_card, origin, target, flags) {}

        struct most_wanted_timer : request_timer {
            explicit most_wanted_timer(request_most_wanted *request)
                : request_timer(request, request->target->m_game->m_options.escape_timer) {}
            
            void on_finished() override {
                resolve_most_wanted(request->origin_card, request->origin, request->target);
            }
        };

        std::optional<most_wanted_timer> m_timer{this};
        request_timer *timer() override { return m_timer ? &*m_timer : nullptr; }
        
        void on_update() override {
            if (target->immune_to(origin_card, origin, flags)) {
                target->m_game->pop_request();
            } else {
                switch (target->can_escape(origin, origin_card, flags)) {
                case 0:
                    auto_pick();
                    break;
                case 2:
                    m_timer.reset();
                }
            }
        }

        bool can_pick(const card *target_card) const override {
            if (target->m_game->m_deck.empty()) {
                return target_card->pocket == pocket_type::discard_pile;
            } else {
                return target_card->pocket == pocket_type::main_deck;
            }
        }

        void on_pick(card *target_card) override {
            origin->m_game->pop_request();
            resolve_most_wanted(origin_card, origin, target);
        }

        game_string status_text(player *owner) const override {
            if (owner == target) {
                return {"STATUS_MOST_WANTED", origin_card};
            } else {
                return {"STATUS_MOST_WANTED_OTHER", target, origin_card};
            }
        }
    };

    void effect_most_wanted::on_play(card *origin_card, player *origin, player *target, effect_flags flags) {
        origin->m_game->queue_request<request_most_wanted>(origin_card, origin, target, flags);
    }
}