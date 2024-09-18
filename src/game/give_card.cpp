#include "give_card.h"

#include "game.h"

namespace banggame {

    bool give_card(game *game, player_ptr target, std::string_view card_name) {
        auto all_cards = game->get_all_cards();
        auto card_it = rn::find_if(all_cards, [&](const_card_ptr target_card) {
            if (rn::equal(card_name, target_card->name, {}, toupper, toupper)) {
                switch (target_card->deck) {
                case card_deck_type::train:
                case card_deck_type::main_deck:
                    return target_card->pocket != pocket_type::player_hand || target_card->owner != target;
                case card_deck_type::character:
                    return target_card->pocket != pocket_type::player_character && target_card->pocket != pocket_type::player_backup;
                case card_deck_type::goldrush:
                    return target_card->pocket != pocket_type::shop_selection && target_card->pocket != pocket_type::hidden_deck
                        && (target_card->pocket != pocket_type::player_table || target_card->owner != target);
                case card_deck_type::highnoon:
                case card_deck_type::fistfulofcards:
                    return game->m_scenario_cards.empty() || target_card != game->m_scenario_cards.back();
                case card_deck_type::wildwestshow:
                    return game->m_wws_scenario_cards.empty() || target_card != game->m_wws_scenario_cards.back();
                }
            }
            return false;
        });
        if (card_it == all_cards.end()) {
            return false;
        }
        card_ptr target_card = *card_it;
        
        game->send_request_status_clear();
        
        switch (target_card->deck) {
        case card_deck_type::main_deck: {
            if (target_card->owner) {
                target->steal_card(target_card);
            } else {
                target->add_to_hand(target_card);
            }
            break;
        }
        case card_deck_type::character: {
            target->remove_extra_characters();
            for (card_ptr c : target->m_characters) {
                target->disable_equip(c);
            }

            card_ptr old_character = target->first_character();
            int ncubes = old_character->num_cubes;

            old_character->move_cubes(nullptr, ncubes);
            game->add_update<"remove_cards">(std::vector{old_character});

            old_character->pocket = pocket_type::none;
            old_character->owner = nullptr;
            old_character->visibility = card_visibility::hidden;

            target->m_characters.clear();
            target->m_characters.push_back(target_card);

            target_card->pocket = pocket_type::player_character;
            target_card->owner = target;

            game->add_update<"add_cards">(target_card, pocket_type::player_character, target);
            target_card->set_visibility(card_visibility::shown, nullptr, true);

            target->reset_max_hp();
            target->enable_equip(target_card);
            target_card->add_cubes(ncubes);
            break;
        }
        case card_deck_type::goldrush: {
            game->m_shop_selection.front()->move_to(pocket_type::shop_discard);
            target_card->move_to(pocket_type::shop_selection);
            break;
        }
        case card_deck_type::highnoon:
        case card_deck_type::fistfulofcards: {
            if (target_card->pocket == pocket_type::scenario_deck) {
                if (auto it = rn::find(game->m_scenario_deck, target_card); it != game->m_scenario_deck.end()) {
                    game->m_scenario_deck.erase(it);
                } else {
                    game->add_update<"add_cards">(target_card, pocket_type::scenario_deck);
                }
                game->m_scenario_deck.push_back(target_card);
                target_card->set_visibility(card_visibility::shown, nullptr, true);
                game->add_update<"move_card">(target_card, nullptr, pocket_type::scenario_deck, 0ms);
            } else {
                target_card->move_to(pocket_type::scenario_deck);
            }
            break;
        }
        case card_deck_type::wildwestshow: {
            if (target_card->pocket == pocket_type::wws_scenario_deck) {
                if (auto it = rn::find(game->m_wws_scenario_deck, target_card); it != game->m_wws_scenario_deck.end()) {
                    game->m_wws_scenario_deck.erase(it);
                } else {
                    game->add_update<"add_cards">(target_card, pocket_type::wws_scenario_deck);
                }
                game->m_wws_scenario_deck.push_back(target_card);
                target_card->set_visibility(card_visibility::shown, nullptr, true);
                game->add_update<"move_card">(target_card, nullptr, pocket_type::wws_scenario_deck, 0ms);
            } else {
                target_card->move_to(pocket_type::wws_scenario_deck);
            }
            break;
        }
        case card_deck_type::train: {
            if (target_card->owner) {
                target->steal_card(target_card);
            } else {
                bool from_train = target_card->pocket == pocket_type::train;
                target->equip_card(target_card);
                if (card_ptr drawn_card = game->top_train_card(); from_train && drawn_card) {
                    drawn_card->move_to(pocket_type::train);
                }
            }
            break;
        }
        }

        game->commit_updates();
        return true;
    }
}