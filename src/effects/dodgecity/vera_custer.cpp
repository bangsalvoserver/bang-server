#include "vera_custer.h"

#include "cards/filter_enums.h"

#include "effects/base/pick.h"

#include "game/game.h"

namespace banggame {

    struct request_vera_custer : request_picking_player {
        request_vera_custer(card_ptr origin_card, player_ptr target)
            : request_picking_player(origin_card, nullptr, target, {}, -25) {}

        void on_update() override {
            if (target->alive() && target->m_game->m_playing == target && !target->m_game->is_disabled(origin_card)) {
                auto_pick();
            } else {
                target->m_game->pop_request();
                target->remove_extra_characters();
            }
        }

        bool can_pick(const_player_ptr target_player) const override {
            return target_player != target;
        }

        void on_pick(player_ptr target_player) {
            target->m_game->pop_request();

            auto new_cards = target_player->m_characters
                | rv::take_last(2)
                | rv::transform([](const_card_ptr target_card) {
                    for (card_ptr c : target_card->m_game->get_all_cards()) {
                        if (c != target_card && c->deck == target_card->deck && c->name == target_card->name) {
                            return c;
                        }
                    }
                    return target_card->m_game->add_card(*target_card);
                })
                | rn::to_vector;

            if (!rn::equal(target->m_characters | rv::drop(1), new_cards)) {
                target->remove_extra_characters();

                for (card_ptr target_card : new_cards) {
                    target->m_game->add_log("LOG_COPY_CHARACTER", target, target_card);

                    target_card->pocket = pocket_type::player_character;
                    target_card->owner = target;
                    
                    target->m_characters.emplace_back(target_card);
                    target->enable_equip(target_card);

                    target->m_game->add_update<"add_cards">(target_card, pocket_type::player_character, target);
                    target_card->set_visibility(card_visibility::shown, nullptr, true);
                }
            }
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_VERA_CUSTER", origin_card};
            } else {
                return {"STATUS_VERA_CUSTER_OTHER", target, origin_card};
            }
        }
    };

    void equip_vera_custer::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::pre_turn_start>(origin_card, [=](player_ptr target) {
            if (origin == target) {
                origin->m_game->queue_request<request_vera_custer>(origin_card, target);
            }
        });
    }

}