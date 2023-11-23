#include "newidentity.h"

#include "game/game.h"

namespace banggame {

    struct request_newidentity : request_base {
        request_newidentity(card *origin_card, player *target)
            : request_base(origin_card, nullptr, target, {}, -7) {}

        void on_update() override {
            if (target->alive() && target->m_game->m_playing == target) {
                if (!live) {
                    target->m_game->move_card(target->m_backup_character.front(), pocket_type::selection);
                }
            } else {
                target->m_game->pop_request();
            }
        }

        bool can_pick(card *target_card) const override {
            return target_card->pocket == pocket_type::selection
                || (target_card->pocket == pocket_type::player_character && target_card == target->first_character());
        }

        void on_pick(card *target_card) override {
            target->m_game->pop_request();
            if (target_card->pocket == pocket_type::selection) {
                target->remove_extra_characters();
                for (card *c : target->m_characters) {
                    target->disable_equip(c);
                }

                target->m_game->add_log("LOG_CHARACTER_CHOICE", target, target_card);

                card *old_character = target->first_character();
                target->m_game->set_card_visibility(old_character, target, card_visibility::hidden);
                target->m_game->move_card(old_character, pocket_type::player_backup, target, card_visibility::hidden, true);
                target->m_game->move_card(target_card, pocket_type::player_character, target, card_visibility::shown);
                target->m_game->move_cubes(old_character, target_card, old_character->num_cubes, true);

                target->reset_max_hp();
                target->enable_equip(target_card);
                
                if (!target->is_ghost()) {
                    target->set_hp(2);
                }
            } else {
                target->m_game->move_card(target->m_game->m_selection.front(), pocket_type::player_backup, target, card_visibility::hidden);
            }
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_NEWIDENTITY", origin_card};
            } else {
                return {"STATUS_NEWIDENTITY_OTHER", target, origin_card};
            }
        }
    };

    void equip_newidentity::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_turn_switch>(target_card, [=](player *origin) {
            target->m_game->queue_request<request_newidentity>(target_card, origin);
        });
    }
}