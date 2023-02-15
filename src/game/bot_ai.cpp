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

    struct random_target_visitor {
        player *origin;
        card *origin_card;
        const effect_holder &holder;
        const effect_context &ctx;

        player *operator()(enums::enum_tag_t<target_type::player>) const {
            return random_element(make_player_target_set(origin, origin_card, holder, ctx), origin->m_game->rng);
        }

        player *operator()(enums::enum_tag_t<target_type::conditional_player>) const {
            auto targets = ranges::to<std::vector>(make_player_target_set(origin, origin_card, holder, ctx));
            if (targets.empty()) {
                return nullptr;
            } else {
                return random_element(targets, origin->m_game->rng);
            }
        }

        card *operator()(enums::enum_tag_t<target_type::card>) const {
            auto targets = ranges::to<std::vector>(make_card_target_set(origin, origin_card, holder, ctx));
            return random_element(targets, origin->m_game->rng);
        }

        card *operator()(enums::enum_tag_t<target_type::extra_card> tag) const {
            if (origin_card == origin->get_last_played_card()) {
                return nullptr;
            } else {
                auto targets = ranges::to<std::vector>(make_card_target_set(origin, origin_card, holder, ctx));
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
            auto targets = ranges::to<std::vector>(make_card_target_set(origin, origin_card, holder, ctx));
            return targets
                | ranges::views::sample(holder.target_value, origin->m_game->rng)
                | ranges::to<std::vector<not_null<card *>>>;
        }

        auto operator()(enums::enum_tag_t<target_type::cards_other_players>) const {
            return range_other_players(origin)
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
                | ranges::views::for_each([](card *slot) {
                    return ranges::views::repeat_n(slot, slot->num_cubes);
                })
                | ranges::to<std::vector>;
            return cubes
                | ranges::views::sample(holder.target_value, origin->m_game->rng)
                | ranges::to<std::vector<not_null<card *>>>;
        }
    };

    static play_card_target generate_random_target(player *origin, card *origin_card, const effect_holder &holder, const effect_context &ctx) {
        return enums::visit_enum([&]<target_type E>(enums::enum_tag_t<E> tag) -> play_card_target {
            if constexpr (play_card_target::has_type<E>) {
                return {tag, random_target_visitor{origin, origin_card, holder, ctx}(tag)};
            } else {
                return tag;
            }
        }, holder.target);
    }

    static card *random_card_playable_with_modifiers(player *origin, bool is_response, const std::vector<card *> &modifiers, const effect_context &ctx) {
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
            if (target_card->is_modifier()) {
                if (ranges::contains(modifiers, target_card)) {
                    return false;
                }
                if (!std::transform_reduce(
                    modifiers.begin(), modifiers.end(), modifier_bitset(target_card->modifier_type()), std::bit_and(),
                    [](card *mod) { return allowed_modifiers_after(mod->modifier_type()); }
                )) {
                    return false;
                }
            }
            return std::ranges::all_of(modifiers, [&](card *mod) { return allowed_card_with_modifier(origin, mod, target_card); })
                && is_possible_to_play(origin, target_card, is_response ? effect_list_index::responses : effect_list_index::effects, ctx);
        });

        if (cards.empty()) {
            return nullptr;
        }

        return random_element(cards, origin->m_game->rng);
    }

    static play_card_verify generate_random_play(player *origin, card *origin_card, bool is_response, std::vector<card *> modifiers = {}, effect_context ctx = {}) {
        if (!is_response && (origin_card->pocket == pocket_type::player_hand || origin_card->pocket == pocket_type::shop_selection) && !origin_card->is_brown()) {
            play_card_verify verifier { origin, origin_card };
            if (!origin_card->self_equippable()) {
                verifier.targets.emplace_back(enums::enum_tag<target_type::player>,
                    random_element(make_equip_set(origin, origin_card), origin->m_game->rng));
            }
            return verifier;
        } else if (origin_card->is_modifier()) {
            modifiers.push_back(origin_card);
            origin_card->modifier.add_context(origin_card, origin, ctx);
            origin_card = random_card_playable_with_modifiers(origin, is_response, modifiers, ctx);
            if (!origin_card) {
                return play_card_verify{};
            }
            return generate_random_play(origin, origin_card, is_response, std::move(modifiers), ctx);
        } else {
            play_card_verify verifier { origin, origin_card, is_response };
            for (card *mod_card : modifiers) {
                verifier.modifiers.emplace_back(mod_card,
                    (is_response ? mod_card->responses : mod_card->effects)
                        | ranges::views::transform([&](const effect_holder &holder) {
                            auto target = generate_random_target(origin, mod_card, holder, ctx);
                            if (holder.type == effect_type::ctx_add) {
                                if (target.is(target_type::card)) {
                                    mod_card->modifier.add_context(mod_card, origin, target.get<target_type::card>(), ctx);
                                } else if (target.is(target_type::player)) {
                                    mod_card->modifier.add_context(mod_card, origin, target.get<target_type::player>(), ctx);
                                }
                            }
                            return target;
                        })
                        | ranges::to<target_list>
                );
            }
            for (const effect_holder &holder : is_response ? verifier.origin_card->responses : verifier.origin_card->effects) {
                verifier.targets.push_back(generate_random_target(origin, verifier.origin_card, holder, ctx));
            }
            if (is_possible_to_play(origin, origin_card, effect_list_index::optionals, ctx)) {
                for (const effect_holder &holder : verifier.origin_card->optionals) {
                    verifier.targets.push_back(generate_random_target(origin, verifier.origin_card, holder, ctx));
                }
            }
            return verifier;
        }
    }

    static bool execute_random_play(player *origin, bool is_response, std::set<card *> const& cards, std::initializer_list<pocket_type> pockets) {
        for (int i=0; i<10; ++i) {
            std::set<card *> card_set = cards;
            while (!card_set.empty()) {
                card *origin_card = [&]{
                    for (pocket_type pocket : pockets) {
                        if (auto filter = card_set | std::views::filter([&](card *c) { return c->pocket == pocket; })) {
                            return random_element(filter, origin->m_game->rng);
                        }
                    }
                    return random_element(card_set, origin->m_game->rng);
                }();

                card_set.erase(origin_card);
                auto verifier = generate_random_play(origin, origin_card, is_response);

                if (verifier.origin_card && !verifier.verify_and_play()) {
                    if (auto &prompt = origin->m_prompt) {
                        auto fun = std::move(prompt->first);
                        prompt.reset();
                        if (card_set.empty() && i>=5) {
                            origin->m_game->invoke_action(std::move(fun));
                            return true;
                        }
                    } else {
                        return true;
                    }
                }
            }
        }

        // softlock
        std::cout << "BOT ERROR: could not find card in execute_random_play\n";
        return false;
    }

    static bool respond_to_request(player *origin) {
        auto update = origin->m_game->make_request_update(origin);

        if (!update.pick_cards.empty() && std::ranges::all_of(update.respond_cards, [](card *c) { return c->pocket == pocket_type::button_row; })) {
            origin->m_game->invoke_action([&]{
                auto req = origin->m_game->top_request();
                req->on_pick(random_element(update.pick_cards, origin->m_game->rng));
            });
            return true;
        } else if (!update.respond_cards.empty()) {
            return execute_random_play(origin, true,
                ranges::to<std::set<card *>>(update.respond_cards),
                {
                    pocket_type::player_character,
                    pocket_type::player_table,
                    pocket_type::player_hand
                }
            );
        }
        return false;
    }

    static bool play_in_turn(player *origin) {
        return execute_random_play(origin, false,
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
                    return is_possible_to_play(origin, target_card);
                } else {
                    return contains_at_least(make_equip_set(origin, target_card), 1);
                }
            })
            | ranges::to<std::set>,
            {
                pocket_type::scenario_card,
                pocket_type::wws_scenario_card,
                pocket_type::player_table,
                pocket_type::player_hand,
                pocket_type::player_character,
                pocket_type::shop_selection
            }
        );
    }
    
    struct bot_delay_request : request_base {
        bot_delay_request(player *origin)
            : request_base(nullptr, nullptr, nullptr)
            , m_timer(this, origin) {}
        
        struct bot_delay_timer : request_timer {
            bot_delay_timer(bot_delay_request *request, player *origin)
                : request_timer(request, 0ms)
                , origin(origin) {}
            
            player *origin;

            void on_finished() override {
                play_in_turn(origin);
            }
        };
        
        bot_delay_timer m_timer;
        request_timer *timer() override { return &m_timer; }
    };

    static constexpr size_t game_max_updates = 5000;

    bool game::request_bot_play(player *origin, bool is_response) {
        if (is_response) {
            return respond_to_request(origin);
        } else if (origin->m_game->m_updates.size() > game_max_updates) {
            queue_request_front<bot_delay_request>(origin);
            return true;
        } else {
            return play_in_turn(origin);
        }
    }
}