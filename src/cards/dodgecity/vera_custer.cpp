#include "vera_custer.h"

#include "game/game.h"

namespace banggame {

    static void copy_characters(player *origin, player *target) {
        origin->remove_extra_characters();

        for (card *target_card : target->m_characters | std::views::reverse | std::views::take(2) | std::views::reverse) {
            origin->m_game->add_log("LOG_COPY_CHARACTER", origin, target_card);
            
            card *new_card = &origin->m_game->m_context.cards.emplace(int(origin->m_game->m_context.cards.first_available_id()), *target_card);
            new_card->pocket = pocket_type::player_character;
            new_card->owner = origin;
            
            new_card->tags.push_back(tag_holder{ .type = tag_type::temp_card });
            
            origin->m_characters.emplace_back(new_card);
            new_card->on_enable(origin);

            origin->m_game->add_update<game_update_type::add_cards>(
                make_id_vector(std::views::single(new_card)), pocket_type::player_character, origin);
            origin->m_game->set_card_visibility(new_card, nullptr, card_visibility::shown, true);
        }
    }

    struct request_vera_custer : request_base {
        request_vera_custer(card *origin_card, player *target)
            : request_base(origin_card, nullptr, target) {}

        void on_update() override {
            if (target->m_game->num_alive() == 2) {
                target->m_game->invoke_action([&]{
                    target->m_game->pop_request();
                    copy_characters(target, *std::next(player_iterator(target)));
                });
            }
        }
        
        bool can_pick(card *target_card) const override {
            return target_card->pocket == pocket_type::player_character
                && target_card->owner->alive() && target_card->owner != target;
        }

        void on_pick(card *target_card) override {
            target->m_game->invoke_action([&]{
                target->m_game->pop_request();
                copy_characters(target, target_card->owner);
            });
        }

        game_string status_text(player *owner) const override {
            if (owner == target) {
                return {"STATUS_VERA_CUSTER", origin_card};
            } else {
                return {"STATUS_VERA_CUSTER_OTHER", target, origin_card};
            }
        }
    };

    void equip_vera_custer::on_enable(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::on_turn_start>({origin_card, 1}, [=](player *target) {
            if (origin == target) {
                origin->m_game->queue_request<request_vera_custer>(origin_card, target);
            }
        });
        origin->m_game->add_listener<event_type::on_turn_end>(origin_card, [=](player *target, bool skipped) {
            if (skipped && origin == target && origin->m_num_drawn_cards == 0) {
                origin->remove_extra_characters();
            }
        });
    }

}