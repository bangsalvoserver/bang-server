#include "game.h"
#include "play_verify.h"
#include "possible_to_play.h"

namespace banggame {

    template<std::ranges::range Range, typename Rng>
    decltype(auto) random_element(Range &&range, Rng &rng) {
        if constexpr (std::ranges::contiguous_range<Range>) {
            std::uniform_int_distribution dist{size_t(0), std::ranges::size(range) - 1};
            return range[dist(rng)];
        } else {
            std::uniform_int_distribution dist{size_t(0), size_t(std::ranges::distance(range)) - 1};
            return *std::next(std::ranges::begin(range), dist(rng));
        }
    }

    card *random_card_of_pocket(const auto &range, auto &rng, pocket_type pocket, std::same_as<pocket_type> auto ... rest) {
        if (auto filter = range | std::views::filter([&](card *c) { return c->pocket == pocket; })) {
            return random_element(filter, rng);
        } else if constexpr (sizeof...(rest) != 0) {
            if (card *c = random_card_of_pocket(range, rng, rest...)) {
                return c;
            } else if (auto filter = range | std::views::filter([&](card *c) {
                return c->pocket != pocket && ((c->pocket != rest) && ... && true);
            })) {
                return random_element(filter, rng);
            } else {
                return nullptr;
            }
        } else {
            return nullptr;
        }
    }

    struct random_target_visitor {
        player *origin;
        card *origin_card;
        const effect_holder &holder;

        player *operator()(enums::enum_tag_t<target_type::player>) const {
            return random_element(make_player_target_set(origin, origin_card, holder), origin->m_game->rng);
        }

        player *operator()(enums::enum_tag_t<target_type::conditional_player>) const {
            auto targets = ranges::to<std::vector>(make_player_target_set(origin, origin_card, holder));
            if (targets.empty()) {
                return nullptr;
            } else {
                return random_element(targets, origin->m_game->rng);
            }
        }

        card *operator()(enums::enum_tag_t<target_type::card>) const {
            auto targets = ranges::to<std::vector>(make_card_target_set(origin, origin_card, holder));
            return random_element(targets, origin->m_game->rng);
        }

        card *operator()(enums::enum_tag_t<target_type::extra_card> tag) const {
            if (origin_card == origin->get_last_played_card()) {
                return nullptr;
            } else {
                auto targets = ranges::to<std::vector>(make_card_target_set(origin, origin_card, holder));
                return random_element(targets, origin->m_game->rng);
            }
        }

        auto operator()(enums::enum_tag_t<target_type::fanning_targets>) const {
            std::vector<std::pair<player *, player *>> possible_targets;
            for (player *target1 : origin->m_game->m_players) {
                if (origin != target1 && origin->m_game->calc_distance(origin, target1) <= origin->m_weapon_range + origin->m_range_mod) {
                    if (player *target2 = *std::next(player_iterator(target1)); origin != target2 && target2->m_distance_mod == 0) {
                        possible_targets.emplace_back(target1, target2);
                    }
                    if (player *target2 = *std::prev(player_iterator(target1)); origin != target2 && target2->m_distance_mod == 0) {
                        possible_targets.emplace_back(target1, target2);
                    }
                }
            }
            auto [target1, target2] = random_element(possible_targets, origin->m_game->rng);
            return std::vector<not_null<player *>>{target1, target2};
        }

        auto operator()(enums::enum_tag_t<target_type::cards> tag) const {
            auto targets = ranges::to<std::vector>(make_card_target_set(origin, origin_card, holder));
            return targets
                | ranges::views::sample(holder.target_value, origin->m_game->rng)
                | ranges::to<std::vector<not_null<card *>>>;
        }

        auto operator()(enums::enum_tag_t<target_type::cards_other_players>) const {
            return origin->m_game->m_players
                | ranges::views::filter([&](player *target) {
                    return target != origin && target->alive();
                })
                | ranges::views::transform([](player *target) {
                    return ranges::views::concat(
                        target->m_table | ranges::views::remove_if(&card::is_black),
                        target->m_hand | ranges::views::take(1)
                    );
                })
                | ranges::views::remove_if(ranges::empty)
                | ranges::views::transform([&](auto &&range) {
                    return random_element(range, origin->m_game->rng);
                })
                | ranges::to<std::vector<not_null<card *>>>;
        }

        auto operator()(enums::enum_tag_t<target_type::select_cubes>) const {
            auto cubes = origin->cube_slots()
                | ranges::views::transform([](card *slot) {
                    return ranges::views::repeat_n(slot, slot->num_cubes);
                })
                | ranges::views::join
                | ranges::to<std::vector>;
            return cubes
                | ranges::views::sample(holder.target_value, origin->m_game->rng)
                | ranges::to<std::vector<not_null<card *>>>;
        }
    };

    static play_card_target generate_random_target(player *origin, card *origin_card, const effect_holder &holder) {
        return enums::visit_enum([&]<target_type E>(enums::enum_tag_t<E> tag) -> play_card_target {
            if constexpr (play_card_target::has_type<E>) {
                return {tag, random_target_visitor{origin, origin_card, holder}(tag)};
            } else {
                return tag;
            }
        }, holder.target);
    }

    static card *random_card_playable_with_modifiers(player *origin, bool is_response, const std::vector<card *> &modifiers) {
        auto cards = ranges::views::concat(
            origin->m_characters,
            origin->m_table | ranges::views::remove_if(&card::inactive),
            origin->m_hand,
            origin->m_game->m_shop_selection,
            origin->m_game->m_hidden_deck,
            origin->m_game->m_scenario_cards | ranges::views::take_last(1),
            origin->m_game->m_wws_scenario_cards | ranges::views::take_last(1)
        )
        | ranges::views::filter([&](card *target_card) {
            if (ranges::contains(modifiers, target_card)) return false;
            if (!is_possible_to_play(origin, target_card, is_response ? target_card->responses : target_card->effects)) return false;

            return (target_card->modifier_type() == card_modifier_type::none
                || std::transform_reduce(
                    modifiers.begin(), modifiers.end(), modifier_bitset(target_card->modifier_type()), std::bit_and(),
                    [](card *mod) { return allowed_modifiers_after(mod->modifier_type()); }
                ))
                && std::ranges::all_of(modifiers, [&](card *mod) {
                    return allowed_card_with_modifier(origin, mod, target_card);
                });
        });

        if (cards.empty()) {
            return nullptr;
        }

        return random_element(cards, origin->m_game->rng);
    }

    static play_card_verify generate_random_play(player *origin, card *origin_card, bool is_response, std::vector<card *> modifiers = {}) {
        if (!is_response && (origin_card->pocket == pocket_type::player_hand || origin_card->pocket == pocket_type::shop_selection) && !origin_card->is_brown()) {
            play_card_verify verifier { origin, origin_card };
            if (!origin_card->self_equippable()) {
                verifier.targets.emplace_back(enums::enum_tag<target_type::player>,
                    random_element(make_equip_set(origin, origin_card), origin->m_game->rng));
            }
            return verifier;
        } else if (origin_card->modifier_type() != card_modifier_type::none) {
            modifiers.push_back(origin_card);
            origin_card = random_card_playable_with_modifiers(origin, is_response, modifiers);
            if (!origin_card) {
                return play_card_verify{};
            }
            return generate_random_play(origin, origin_card, is_response, std::move(modifiers));
        } else {
            play_card_verify verifier { origin, origin_card, is_response, {}, std::move(modifiers) };
            if (!verifier.playing_card) {
                return play_card_verify{};
            }
            for (const effect_holder &holder : is_response ? verifier.playing_card->responses : verifier.playing_card->effects) {
                verifier.targets.push_back(generate_random_target(origin, verifier.playing_card, holder));
            }
            if (is_possible_to_play(origin, origin_card, origin_card->optionals)) {
                for (const effect_holder &holder : verifier.playing_card->optionals) {
                    verifier.targets.push_back(generate_random_target(origin, verifier.playing_card, holder));
                }
            }
            return verifier;
        }
    }

    static bool generate_random_play_impl(player *origin, bool is_response, std::ranges::range auto &&cards, std::same_as<pocket_type> auto ... pockets) {
        auto card_set = ranges::to<std::set>(FWD(cards));
        while (!card_set.empty()) {
            card *origin_card = random_card_of_pocket(card_set, origin->m_game->rng, pockets ...);

            card_set.erase(origin_card);
            auto verifier = generate_random_play(origin, origin_card, is_response);

            if (verifier.origin_card && !verifier.verify_and_play()) {
                if (auto &prompt = origin->m_prompt) {
                    auto fun = std::move(prompt->first);
                    prompt.reset();
                    if (card_set.empty()) {
                        origin->m_game->invoke_action(std::move(fun));
                        return true;
                    }
                } else {
                    return true;
                }
            }
        }

        // softlock
        std::cout << "BOT ERROR: could not find card in generate_random_play_impl\n";
        return false;
    }

    static bool respond_to_request(player *origin) {
        auto update = origin->m_game->make_request_update(origin);

        if (!update.pick_cards.empty() && std::ranges::all_of(update.respond_cards, [](card *c) { return c->pocket == pocket_type::button_row; })) {
            origin->m_game->top_request().on_pick(random_element(update.pick_cards, origin->m_game->rng));
            return true;
        } else if (!update.respond_cards.empty()) {
            return generate_random_play_impl(origin, true,
                update.respond_cards,
                pocket_type::player_character,
                pocket_type::player_table,
                pocket_type::player_hand
            );
        }
        return false;
    }

    static bool play_in_turn(player *origin) {
        return generate_random_play_impl(origin, false,
            ranges::views::concat(
                origin->m_characters,
                origin->m_table | ranges::views::remove_if(&card::inactive),
                origin->m_hand,
                origin->m_game->m_shop_selection,
                origin->m_game->m_button_row,
                origin->m_game->m_scenario_cards | ranges::views::take_last(1),
                origin->m_game->m_wws_scenario_cards | ranges::views::take_last(1)
            )
            | ranges::views::filter([&](card *target_card){
                if (target_card->pocket != pocket_type::player_hand && target_card->pocket != pocket_type::shop_selection || target_card->is_brown()) {
                    return target_card->modifier_type() != card_modifier_type::none
                        || is_possible_to_play(origin, target_card, target_card->effects);
                } else {
                    return !make_equip_set(origin, target_card).empty();
                }
            }),
            pocket_type::scenario_card,
            pocket_type::wws_scenario_card,
            pocket_type::player_table,
            pocket_type::player_hand,
            pocket_type::player_character,
            pocket_type::shop_selection
        );
    }

    bool game::request_bot_play(player *origin, bool is_response) {
        if (is_response) {
            return respond_to_request(origin);
        } else {
            return play_in_turn(origin);
        }
    }
}