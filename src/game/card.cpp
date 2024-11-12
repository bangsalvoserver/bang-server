#include "card.h"
#include "game.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"

#include "effects/armedanddangerous/ruleset.h"

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

    int card::get_card_cost(const effect_context &ctx) const {
        const_card_ptr target = ctx.card_choice ? ctx.card_choice : this;
        if (!ctx.repeat_card && target->pocket != pocket_type::player_table) {
            return target->get_tag_value(tag_type::buy_cost).value_or(0) - ctx.discount;
        }
        return 0;
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

    void card::move_to(pocket_type new_pocket, player_ptr new_owner, card_visibility new_visibility, bool instant, bool front) {
        if (pocket == new_pocket && owner == new_owner) return;
        
        set_visibility(new_visibility, new_owner, instant);

        auto &prev_pile = m_game->get_pocket(pocket, owner);
        prev_pile.erase(rn::find(prev_pile, this));

        pocket = new_pocket;
        owner = new_owner;

        auto &new_pile = m_game->get_pocket(new_pocket, new_owner);
        if (front) {
            new_pile.insert(new_pile.begin(), this);
        } else {
            new_pile.push_back(this);
        }
        
        m_game->add_update<"move_card">(this, new_owner, new_pocket, instant ? 0ms : durations.move_card, front);
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
    
    void card::add_cubes(int ncubes) {
        ncubes = std::min<int>({ncubes, m_game->num_cubes, max_cubes - num_cubes});
        if (ncubes > 0) {
            m_game->num_cubes -= ncubes;
            num_cubes += ncubes;
            m_game->add_log("LOG_ADD_CUBE", owner, this, ncubes);
            m_game->add_update<"move_cubes">(ncubes, nullptr, this, ncubes == 1 ? durations.move_cube : durations.move_cubes);
        }
    }

    void card::move_cubes(card_ptr target, int ncubes, bool instant) {
        ncubes = std::min<int>(ncubes, num_cubes);
        if (target && ncubes > 0 && target->num_cubes < max_cubes) {
            int added_cubes = std::min<int>(ncubes, max_cubes - target->num_cubes);
            target->num_cubes += added_cubes;
            num_cubes -= added_cubes;
            ncubes -= added_cubes;
            if (owner == target->owner) {
                m_game->add_log("LOG_MOVED_CUBE", target->owner, this, target, added_cubes);
            } else {
                m_game->add_log("LOG_MOVED_CUBE_FROM", target->owner, owner, this, target, added_cubes);
            }
            m_game->add_update<"move_cubes">(added_cubes, this, target, instant ? 0ms : added_cubes == 1 ? durations.move_cube : durations.move_cubes);
        }
        if (ncubes > 0) {
            num_cubes -= ncubes;
            m_game->num_cubes += ncubes;
            m_game->add_log("LOG_PAID_CUBE", owner, this, ncubes);
            m_game->add_update<"move_cubes">(ncubes, this, nullptr, instant ? 0ms : ncubes == 1 ? durations.move_cube : durations.move_cubes);
        }
        if (sign && num_cubes == 0) {
            m_game->add_log("LOG_DISCARDED_ORANGE_CARD", owner, this);
            m_game->call_event(event_type::on_discard_orange_card{ owner, this });
            owner->disable_equip(this);
            move_to(pocket_type::discard_pile);
        }
    }

    void card::add_fame(int fame) {
        if (fame > 0) {
            num_fame += fame;
            m_game->add_update<"add_fame">(fame, this);
        }
    }

    void card::move_fame(card_ptr target, int fame, bool instant) {
        fame = std::min<int>(num_fame, fame);
        if (fame > 0) {
            num_fame -= fame;
            if (target) {
                target->num_fame += fame;
            }
            m_game->add_update<"move_fame">(fame, this, target, instant ? 0ms : durations.move_fame);
        }
    }

    void card::drop_cubes() {
        if (num_cubes > 0) {
            m_game->add_log("LOG_DROP_CUBE", owner, this, num_cubes);
            m_game->num_cubes += num_cubes;
            m_game->add_update<"move_cubes">(num_cubes, this, nullptr, num_cubes == 1 ? durations.move_cube : durations.move_cubes);
            num_cubes = 0;
        }
    }
}