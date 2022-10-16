#include "rust.h"

#include "../../game.h"

namespace banggame {

    static void resolve_rust(card *origin_card, player *origin, player *target) {
        auto view = target->m_table | std::views::filter([](card *c){ return c->color == card_color_type::orange; });
        std::vector<card *> orange_cards{view.begin(), view.end()};
        
        orange_cards.push_back(target->m_characters.front());
        for (card *c : orange_cards) {
            target->move_cubes(c, origin->m_characters.front(), 1);
        }

        target->m_game->call_event<event_type::on_effect_end>(origin, origin_card);
    }

    struct request_rust : request_base, resolvable_request {
        using request_base::request_base;

        void on_resolve() override {
            origin->m_game->pop_request();
            resolve_rust(origin_card, origin, target);
            origin->m_game->update_request();
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_RUST", origin_card};
            } else {
                return {"STATUS_RUST_OTHER", target, origin_card};
            }
        }
    };

    game_string effect_rust::on_prompt(card *origin_card, player *origin, player *target) {
        if (target->count_cubes() == 0) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        } else {
            return {};
        }
    }
    
    void effect_rust::on_play(card *origin_card, player *origin, player *target, effect_flags flags) {
        if (target->count_cubes() == 0) return;
        origin->m_game->queue_action([=]{
            if (target->can_escape(origin, origin_card, flags)) {
                origin->m_game->queue_request<request_rust>(origin_card, origin, target, flags);
            } else {
                resolve_rust(origin_card, origin, target);
            }
        });
    }

}