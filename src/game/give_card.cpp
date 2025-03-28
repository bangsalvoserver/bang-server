#include "give_card.h"

#include "game_table.h"

namespace banggame {

    bool give_card(player_ptr target, std::string_view card_name) {
        auto all_cards = target->m_game->m_cards_storage | rv::addressof;
        auto card_it = rn::find_if(all_cards, [&](const_card_ptr target_card) {
            if (string_equal_icase(card_name, target_card->name)) {
                switch (target_card->deck) {
                case card_deck_type::train:
                case card_deck_type::main_deck:
                    return target_card->pocket != pocket_type::player_hand || target_card->owner != target;
                case card_deck_type::character:
                    return target_card != target->get_character();
                case card_deck_type::goldrush:
                    return target_card->pocket != pocket_type::shop_selection && target_card->pocket != pocket_type::hidden_deck
                        && (target_card->pocket != pocket_type::player_table || target_card->owner != target);
                case card_deck_type::highnoon:
                case card_deck_type::fistfulofcards:
                    return target->m_game->m_scenario_cards.empty() || target_card != target->m_game->m_scenario_cards.back();
                case card_deck_type::wildwestshow:
                    return target->m_game->m_wws_scenario_cards.empty() || target_card != target->m_game->m_wws_scenario_cards.back();
                case card_deck_type::feats:
                    return target_card->pocket != pocket_type::feats;
                }
            }
            return false;
        });
        if (card_it == all_cards.end()) {
            return false;
        }
        card_ptr target_card = *card_it;
        
        target->m_game->clear_request_status();
        
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
            if (target_card->pocket == pocket_type::player_character && target_card->owner != target) {
                card_ptr old_character = target->get_character();
                target->remove_cards(target->m_characters);
                target_card->owner->set_character(old_character);
            }
            target->set_character(target_card);
            break;
        }
        case card_deck_type::goldrush: {
            target->m_game->m_shop_selection.front()->move_to(pocket_type::shop_deck, nullptr, card_visibility::shown, false, pocket_position::begin);
            target_card->move_to(pocket_type::shop_selection);
            break;
        }
        case card_deck_type::highnoon:
        case card_deck_type::fistfulofcards: {
            if (target_card->pocket == pocket_type::scenario_deck) {
                if (auto it = rn::find(target->m_game->m_scenario_deck, target_card); it != target->m_game->m_scenario_deck.end()) {
                    target->m_game->m_scenario_deck.erase(it);
                } else {
                    target->m_game->add_update<"add_cards">(target_card, pocket_type::scenario_deck);
                }
                target->m_game->m_scenario_deck.push_back(target_card);
                target_card->set_visibility(card_visibility::shown, nullptr, true);
                target->m_game->add_update<"move_card">(target_card, nullptr, pocket_type::scenario_deck, 0ms);
            } else {
                target_card->move_to(pocket_type::scenario_deck);
            }
            break;
        }
        case card_deck_type::wildwestshow: {
            if (target_card->pocket == pocket_type::wws_scenario_deck) {
                if (auto it = rn::find(target->m_game->m_wws_scenario_deck, target_card); it != target->m_game->m_wws_scenario_deck.end()) {
                    target->m_game->m_wws_scenario_deck.erase(it);
                } else {
                    target->m_game->add_update<"add_cards">(target_card, pocket_type::wws_scenario_deck);
                }
                target->m_game->m_wws_scenario_deck.push_back(target_card);
                target_card->set_visibility(card_visibility::shown, nullptr, true);
                target->m_game->add_update<"move_card">(target_card, nullptr, pocket_type::wws_scenario_deck, 0ms);
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
                if (from_train && !target->m_game->m_train_deck.empty()) {
                    target->m_game->m_train_deck.back()->move_to(pocket_type::train);
                }
            }
            break;
        }
        case card_deck_type::feats: {
            if (target->m_game->m_feats.size() >= 4) {
                card_ptr last_feat = target->m_game->m_feats.back();
                target->m_game->m_first_player->disable_equip(last_feat);
                last_feat->drop_all_fame();
                last_feat->move_to(pocket_type::feats_discard);
            }
            target_card->move_to(pocket_type::feats);
            target->m_game->m_first_player->enable_equip(target_card);
            break;
        }
        }

        target->m_game->commit_updates();
        return true;
    }
}