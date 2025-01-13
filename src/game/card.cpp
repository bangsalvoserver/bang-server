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

    void card::set_visibility(card_visibility new_visibility, player_ptr new_owner, bool instant) {
        animation_duration duration = instant ? 0ms : durations.flip_card;
        if (new_visibility == card_visibility::hidden) {
            if (visibility == card_visibility::show_owner) {
                m_game->add_update<"hide_card">(update_target::includes(owner), this, duration);
            } else if (visibility == card_visibility::shown) {
                m_game->add_update<"hide_card">(this, duration);
            }
            visibility = card_visibility::hidden;
        } else if (!new_owner || new_visibility == card_visibility::shown) {
            if (visibility == card_visibility::show_owner) {
                m_game->add_update<"show_card">(update_target::excludes(owner), this, *this, duration);
            } else if (visibility == card_visibility::hidden) {
                m_game->add_update<"show_card">(this, *this, duration);
            }
            visibility = card_visibility::shown;
        } else if (owner != new_owner || visibility != card_visibility::show_owner) {
            if (visibility == card_visibility::shown) {
                m_game->add_update<"hide_card">(update_target::excludes(new_owner), this, duration);
            } else {
                if (visibility == card_visibility::show_owner) {
                    m_game->add_update<"hide_card">(update_target::includes(owner), this, duration);
                }
                m_game->add_update<"show_card">(update_target::includes(new_owner), this, *this, duration);
            }
            visibility = card_visibility::show_owner;
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
        
        m_game->add_update<"move_card">(this, new_owner, new_pocket, instant ? 0ms : durations.move_card, position);
    }

    void card::set_inactive(bool new_inactive) {
        if (new_inactive != inactive) {
            m_game->add_update<"tap_card">(this, new_inactive);
            inactive = new_inactive;
        }
    }

    void card::flash_card() {
        m_game->add_update<"flash_card">(this);
    }

    void card::add_short_pause() {
        m_game->add_update<"short_pause">(this);
    }

    int card::num_tokens(card_token_type token_type) const {
        return tokens[token_type];
    }
    
    void card::add_tokens(card_token_type token_type, int num_tokens) {
        auto &table_tokens = m_game->tokens[token_type];
        auto &card_tokens = tokens[token_type];

        if (table_tokens < num_tokens) {
            num_tokens = table_tokens;
        }
        if (token_type == card_token_type::cube) {
            num_tokens = std::min<int>(num_tokens, max_cubes - card_tokens);
        }

        if (num_tokens > 0) {
            table_tokens -= num_tokens;
            card_tokens += num_tokens;
            if (token_type == card_token_type::cube) {
                m_game->add_log("LOG_ADD_CUBE", owner, this, num_tokens);
            }
            m_game->add_update<"move_tokens">(token_type, num_tokens, nullptr, this, num_tokens == 1 ? durations.move_token : durations.move_tokens);
        }
    }

    void card::move_tokens(card_token_type token_type, card_ptr target, int num_tokens, bool instant) {
        auto &table_tokens = m_game->tokens[token_type];
        auto &target_tokens = target ? target->tokens[token_type] : table_tokens;
        auto &card_tokens = tokens[token_type];

        if (card_tokens < num_tokens) {
            num_tokens = card_tokens;
        }
        if (target && num_tokens > 0 && (token_type != card_token_type::cube || target_tokens < max_cubes)) {
            int added_tokens = token_type == card_token_type::cube
                ? std::min<int>(num_tokens, max_cubes - target_tokens)
                : num_tokens;
            target_tokens += added_tokens;
            card_tokens -= added_tokens;
            num_tokens -= added_tokens;
            if (token_type == card_token_type::cube) {
                if (owner == target->owner) {
                    m_game->add_log("LOG_MOVED_CUBE", target->owner, this, target, added_tokens);
                } else {
                    m_game->add_log("LOG_MOVED_CUBE_FROM", target->owner, owner, this, target, added_tokens);
                }
            }
            m_game->add_update<"move_tokens">(token_type, added_tokens, this, target, instant ? 0ms : num_tokens == 1 ? durations.move_token : durations.move_tokens);
        }
        if (num_tokens > 0) {
            card_tokens -= num_tokens;
            table_tokens += num_tokens;
            if (token_type == card_token_type::cube) {
                m_game->add_log("LOG_PAID_CUBE", owner, this, num_tokens);
            }
            m_game->add_update<"move_tokens">(token_type, num_tokens, this, nullptr, instant ? 0ms : num_tokens == 1 ? durations.move_token : durations.move_tokens);
        }
        if (card_tokens == 0) {
            m_game->call_event(event_type::on_finish_tokens{ this, target, token_type });
        }
    }

    void card::drop_all_cubes() {
        if (auto &count = tokens[card_token_type::cube]) {
            m_game->add_log("LOG_DROP_CUBE", owner, this, count);
            m_game->tokens[card_token_type::cube] += count;
            m_game->add_update<"move_tokens">(card_token_type::cube, count, this, nullptr, count == 1 ? durations.move_token : durations.move_tokens);
            count = 0;
        }
    }

    void card::drop_all_fame() {
        for (auto [token, count] : tokens) {
            if (token != card_token_type::cube && count > 0) {
                m_game->add_update<"add_tokens">(token, -count, this);
                count = 0;
            }
        }
    }
}