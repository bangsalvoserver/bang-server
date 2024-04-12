#include "greygory_deck.h"

#include "game/game.h"

#include "effects/base/resolve.h"

namespace banggame {
    
    static void greygory_deck_add_characters(card *target_card, player *target) {
        std::array<card *, 2> base_characters;
        rn::sample(target->m_game->context().cards
            | rv::transform([](card &c) { return &c; })
            | rv::filter([&](card *c) {
                return c != target_card && c->expansion == expansion_type{}
                    && (c->pocket == pocket_type::none
                    || (c->pocket == pocket_type::player_character && c->owner == target));
            }),
            base_characters.begin(), base_characters.size(), target->m_game->rng);

        target->m_game->add_update<game_update_type::add_cards>(rn::to<std::vector<card_backface>>(base_characters), pocket_type::player_character, target);
        for (card *c : base_characters) {
            target->m_characters.push_back(c);
            target->m_game->add_log("LOG_CHARACTER_CHOICE", target, c);
            c->pocket = pocket_type::player_character;
            c->owner = target;
            target->enable_equip(c);
            target->m_game->set_card_visibility(c, nullptr, card_visibility::shown, true);
        }
    }

    struct request_greygory_deck : request_resolvable {
        request_greygory_deck(card *origin_card, player *origin)
            : request_resolvable(origin_card, nullptr, origin) {}

        void on_update() override {
            auto_resolve();
        }

        void on_resolve() override {
            target->m_game->pop_request();
        }
        
        game_string status_text(player *owner) const {
            if (target == owner) {
                return {"STATUS_CAN_PLAY_CARD", origin_card};
            } else {
                return {"STATUS_CAN_PLAY_CARD_OTHER", target, origin_card};
            }
        }
    };

    void equip_greygory_deck::on_enable(card *target_card, player *target) {
        if (target->m_characters.size() == 1) {
            greygory_deck_add_characters(target_card, target);
        }
        target->m_game->add_listener<event_type::on_turn_start>(target_card, [=](player *origin) {
            if (origin == target) {
                target->m_game->queue_request<request_greygory_deck>(target_card, target);
            }
        });
    }

    bool effect_greygory_deck::can_play(card *origin_card, player *origin) {
        return origin->m_game->top_request<request_greygory_deck>(origin) != nullptr;
    }
    
    void effect_greygory_deck::on_play(card *origin_card, player *origin) {
        origin->m_game->pop_request();
        origin->remove_extra_characters();
        greygory_deck_add_characters(origin_card, origin);
    }
}