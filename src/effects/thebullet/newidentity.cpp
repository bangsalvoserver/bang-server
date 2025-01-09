#include "newidentity.h"

#include "effects/base/pick.h"

#include "game/game_table.h"

namespace banggame {

    struct request_newidentity : request_picking {
        request_newidentity(card_ptr origin_card, player_ptr target)
            : request_picking(origin_card, nullptr, target, {}, -20) {}

        void on_update() override {
            if (target->alive() && target->m_game->m_playing == target) {
                if (!live) {
                    target->m_backup_character.front()->move_to(pocket_type::selection);
                }
            } else {
                target->m_game->pop_request();
            }
        }

        bool can_pick(const_card_ptr target_card) const override {
            return target_card->pocket == pocket_type::selection
                || (target_card->pocket == pocket_type::player_character && target_card == target->first_character());
        }

        void on_pick(card_ptr target_card) override {
            target->m_game->pop_request();
            if (target_card->pocket == pocket_type::selection) {
                target->remove_extra_characters();
                for (card_ptr c : target->m_characters) {
                    target->disable_equip(c);
                }

                target->m_game->add_log("LOG_CHARACTER_CHOICE", target, target_card);

                card_ptr old_character = target->first_character();
                old_character->set_visibility(card_visibility::hidden, target);
                old_character->move_to(pocket_type::player_backup, target, card_visibility::hidden, true);
                target_card->move_to(pocket_type::player_character, target, card_visibility::shown);
                
                for (const auto &[token, count] : old_character->tokens) {
                    old_character->move_tokens(token, target_card, count, true);
                }

                target->reset_max_hp();
                target->enable_equip(target_card);
                
                if (!target->is_ghost()) {
                    target->set_hp(2);
                }
            } else {
                target->m_game->m_selection.front()->move_to(pocket_type::player_backup, target, card_visibility::hidden);
            }
        }

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_NEWIDENTITY", origin_card};
            } else {
                return {"STATUS_NEWIDENTITY_OTHER", target, origin_card};
            }
        }
    };

    void equip_newidentity::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_switch>({target_card, -1}, [=](player_ptr origin) {
            target->m_game->queue_request<request_newidentity>(target_card, origin);
        });
    }
}