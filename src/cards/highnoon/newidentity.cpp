#include "newidentity.h"

#include "game/game.h"

namespace banggame {

    struct request_newidentity : request_base {
        request_newidentity(card *origin_card, player *target)
            : request_base(origin_card, nullptr, target) {}

        bool can_pick(card *target_card) const override {
            return target_card->pocket == pocket_type::selection
                || (target_card->pocket == pocket_type::player_character && target_card == target->m_characters.front());
        }

        void on_pick(card *target_card) override {
            auto lock = target->m_game->lock_updates(true);
            if (target_card->pocket == pocket_type::selection) {
                target->remove_extra_characters();
                for (card *c : target->m_characters) {
                    target->disable_equip(c);
                }

                target->m_game->add_log("LOG_CHARACTER_CHOICE", target, target_card);

                card *old_character = target->m_characters.front();
                target->m_game->move_card(old_character, pocket_type::player_backup, target, show_card_flags::hidden);
                target->m_game->move_card(target_card, pocket_type::player_character, target, show_card_flags::shown);

                target->reset_max_hp();
                target->enable_equip(target_card);
                target->move_cubes(old_character, target_card, old_character->num_cubes);
                target_card->on_equip(target);
                
                target->set_hp(2);
            } else {
                target->m_game->move_card(target->m_game->m_selection.front(), pocket_type::player_backup, target, show_card_flags::hidden);
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
        target->m_game->add_listener<event_type::pre_turn_start>(target_card, [=](player *p) {
            target->m_game->move_card(p->m_backup_character.front(), pocket_type::selection);
            target->m_game->queue_request<request_newidentity>(target_card, p);
        });
    }
}