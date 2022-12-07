#include "vera_custer.h"

#include "game/game.h"

namespace banggame {

    static void copy_characters(player *origin, player *target) {
        origin->remove_extra_characters();

        std::ranges::for_each(
            target->m_characters
                | std::views::reverse
                | std::views::take(2)
                | std::views::reverse,
            [origin](card *target_card) {
                origin->m_game->add_log("LOG_COPY_CHARACTER", origin, target_card);
                
                auto card_copy = std::make_unique<card>(static_cast<const card_data &>(*target_card));
                card_copy->id = int(origin->m_game->m_cards.first_available_id());
                card_copy->pocket = pocket_type::player_character;
                card_copy->owner = origin;
                card_copy->tags.push_back(tag_holder{ .type = tag_type::temp_card });

                card *new_card = origin->m_game->m_cards.insert(std::move(card_copy)).get();
                
                origin->m_characters.emplace_back(new_card);
                new_card->on_enable(origin);

                origin->m_game->add_update<game_update_type::add_cards>(
                    make_id_vector(std::views::single(new_card)), pocket_type::player_character, origin);
                origin->m_game->set_card_visibility(new_card, nullptr, card_visibility::shown, true);
            });
    }

    struct request_vera_custer : request_base {
        request_vera_custer(card *origin_card, player *target)
            : request_base(origin_card, nullptr, target) {}

        bool auto_resolve() override {
            if (target->m_game->num_alive() == 2) {
                on_pick((*std::next(player_iterator(target)))->m_characters.front());
                return true;
            } else {
                return false;
            }
        }
        
        bool can_pick(card *target_card) const override {
            return target_card->pocket == pocket_type::player_character
                && target_card->owner->alive() && target_card->owner != target;
        }

        void on_pick(card *target_card) override {
            auto lock = target->m_game->lock_updates(true);
            copy_characters(target, target_card->owner);
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