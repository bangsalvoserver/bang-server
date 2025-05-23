#include "card.h"
#include "game_table.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"
#include "cards/game_events.h"

namespace banggame {

    bool card::is_equip_card() const {
        switch (pocket) {
        case pocket_type::player_hand:
        case pocket_type::shop_selection:
            return !is_brown();
        case pocket_type::train:
            return deck != card_deck_type::locomotive;
        default:
            return false;
        }
    }

    bool card::is_bang_card(const_player_ptr origin) const {
        return origin->check_player_flags(player_flag::treat_any_as_bang)
            || has_tag(tag_type::bangcard)
            || origin->check_player_flags(player_flag::treat_missed_as_bang)
            && has_tag(tag_type::missed);
    }
    
    card_sign card::get_modified_sign() const {
        auto value = sign;
        m_game->call_event(event_type::apply_sign_modifier{ value });
        return value;
    }

    card_visibility card::get_visibility() const {
        if (visibility.exclusive()) {
            return card_visibility::shown;
        } else if (visibility.matches(owner)) {
            return card_visibility::show_owner;
        } else {
            return card_visibility::hidden;
        }
    }

    void card::set_visibility(update_target new_visibility, bool instant) {
        animation_duration duration = instant ? 0ms : durations.flip_card;
        if (visibility.exclusive()) {
            if (new_visibility.exclusive()) {
                if (auto hide_to = new_visibility - visibility) {
                    m_game->add_update(hide_to, game_updates::hide_card{ this, duration });
                }
                if (auto show_to = visibility - new_visibility) {
                    m_game->add_update(show_to, game_updates::show_card{ this, *this, duration });
                }
            } else {
                m_game->add_update(visibility | new_visibility, game_updates::hide_card{ this, duration });
                if (auto show_to = visibility & new_visibility) {
                    m_game->add_update(show_to, game_updates::show_card{ this, *this, duration });
                }
            }
        } else {
            if (new_visibility.exclusive()) {
                if (auto hide_to = visibility & new_visibility) {
                    m_game->add_update(hide_to, game_updates::hide_card{ this, duration });
                }
                m_game->add_update(visibility | new_visibility, game_updates::show_card{ this, *this, duration });
            } else {
                if (auto hide_to = visibility - new_visibility) {
                    m_game->add_update(hide_to, game_updates::hide_card{ this, duration });
                }
                if (auto show_to = new_visibility - visibility) {
                    m_game->add_update(show_to, game_updates::show_card{ this, *this, duration });
                }
            }
        }
        visibility = new_visibility;
    }

    void card::set_visibility(card_visibility new_visibility, player_ptr new_owner, bool instant) {
        if (new_visibility == card_visibility::hidden) {
            set_visibility(update_target::includes(), instant);
        } else if (!new_owner || new_visibility == card_visibility::shown) {
            set_visibility(update_target::excludes(), instant);
        } else {
            set_visibility(update_target::includes(new_owner), instant);
        }
    }

    void card::move_to(pocket_type new_pocket, player_ptr new_owner, card_visibility new_visibility, bool instant, pocket_position position) {
        if (pocket == new_pocket && owner == new_owner && position != pocket_position::random) return;
        
        set_visibility(new_visibility, new_owner, instant);

        auto &prev_pile = m_game->get_pocket(pocket, owner);
        prev_pile.erase(rn::find(prev_pile, this));

        pocket = new_pocket;
        owner = new_owner;

        auto &new_pile = m_game->get_pocket(new_pocket, new_owner);
        switch (position) {
        case pocket_position::begin:
            new_pile.insert(new_pile.begin(), this);
            break;
        case pocket_position::end:
            new_pile.push_back(this);
            break;
        case pocket_position::random: {
            if (new_pile.empty()) {
                new_pile.push_back(this);
            } else {
                std::uniform_int_distribution<size_t> dist{0, new_pile.size() - 1};
                new_pile.insert(new_pile.begin() + dist(m_game->rng), this);
            }
            break;
        }
        }
        
        m_game->add_update(game_updates::move_card{ this, new_owner, new_pocket, instant ? 0ms : durations.move_card, position });
    }

    void card::set_inactive(bool new_inactive) {
        if (new_inactive != inactive) {
            m_game->add_update(game_updates::tap_card{ this, new_inactive });
            inactive = new_inactive;
        }
    }

    void card::flash_card() {
        m_game->add_update(game_updates::flash_card{ this });
    }

    void card::add_short_pause() {
        m_game->add_update(game_updates::short_pause{ this });
    }

    int card::num_tokens(card_token_type token_type) const {
        return tokens[token_type];
    }

    static animation_duration move_token_duration(int num_tokens, bool instant = false) {
        if (instant) {
            return 0ms;
        } else if (num_tokens == 1) { 
            return durations.move_token;
        } else {
            return durations.move_tokens;
        }
    }

    void card::move_tokens(card_token_type token_type, card_ptr target, int num_tokens, bool instant) {
        auto &target_tokens = target ? target->tokens[token_type] : m_game->tokens[token_type];
        auto &card_tokens = tokens[token_type];

        if (card_tokens < num_tokens) {
            num_tokens = card_tokens;
        }
        if (num_tokens > 0) {
            target_tokens += num_tokens;
            card_tokens -= num_tokens;
            
            m_game->add_update(game_updates::move_tokens{ token_type, num_tokens, this, target, move_token_duration(num_tokens, instant) });
        }
    }
    
    void card::add_cubes(int num_tokens) {
        const card_token_type token_type = card_token_type::cube;
        auto &table_tokens = m_game->tokens[token_type];
        auto &card_tokens = tokens[token_type];

        num_tokens = std::min<int>({num_tokens, table_tokens, max_cubes - card_tokens});
        if (num_tokens > 0) {
            table_tokens -= num_tokens;
            card_tokens += num_tokens;

            m_game->add_log("LOG_ADD_CUBE", owner, this, num_tokens);
            m_game->add_update(game_updates::move_tokens{ token_type, num_tokens, nullptr, this, move_token_duration(num_tokens) });
        }
    }

    void card::move_cubes(card_ptr target, int num_tokens, bool instant) {
        const card_token_type token_type = card_token_type::cube;
        auto &table_tokens = m_game->tokens[token_type];
        auto &target_tokens = target ? target->tokens[token_type] : table_tokens;
        auto &card_tokens = tokens[token_type];

        if (card_tokens < num_tokens) {
            num_tokens = card_tokens;
        }
        if (target && num_tokens > 0 && target_tokens < max_cubes) {
            int added_tokens = std::min<int>(num_tokens, max_cubes - target_tokens);
            target_tokens += added_tokens;
            card_tokens -= added_tokens;
            num_tokens -= added_tokens;

            if (owner == target->owner) {
                m_game->add_log("LOG_MOVED_CUBE", target->owner, this, target, added_tokens);
            } else {
                m_game->add_log("LOG_MOVED_CUBE_FROM", target->owner, owner, this, target, added_tokens);
            }
            m_game->add_update(game_updates::move_tokens{ token_type, added_tokens, this, target, move_token_duration(added_tokens, instant) });
        }
        if (num_tokens > 0) {
            card_tokens -= num_tokens;
            table_tokens += num_tokens;
            
            m_game->add_log("LOG_PAID_CUBE", owner, this, num_tokens);
            m_game->add_update(game_updates::move_tokens{ token_type, num_tokens, this, nullptr, move_token_duration(num_tokens, instant) });
        }
        if (card_tokens == 0) {
            m_game->call_event(event_type::on_finish_tokens{ this, target, token_type });
        }
    }

    void card::drop_all_cubes() {
        const card_token_type token_type = card_token_type::cube;
        if (auto &count = tokens[token_type]) {
            m_game->add_log("LOG_DROP_CUBE", owner, this, count);
            m_game->tokens[token_type] += count;
            m_game->add_update(game_updates::move_tokens{ token_type, count, this, nullptr, move_token_duration(count) });
            count = 0;
        }
    }

    void card::drop_all_fame() {
        for (auto [token, count] : tokens) {
            if (is_fame_token(token) && count > 0) {
                m_game->add_update(game_updates::add_tokens{ token, -count, this });
                count = 0;
            }
        }
    }
}