#include "game.h"
#include "play_verify.h"
#include "possible_to_play.h"
#include "filters.h"

namespace banggame {

    template<std::ranges::range Range, typename Rng>
    decltype(auto) random_element(Range &&range, Rng &rng) {
        std::ranges::range_value_t<Range> ret;
        if (std::ranges::sample(std::forward<Range>(range), &ret, 1, rng) == &ret) {
            throw std::runtime_error("empty range in random_element");
        }
        return ret;
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
        auto cards = get_all_active_cards(origin, true)
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
                && is_possible_to_play(origin, target_card, is_response, ctx);
        });

        return random_element(cards, origin->m_game->rng);
    }

    static play_card_args generate_random_play(player *origin, card *origin_card, bool is_response, std::vector<card *> modifier_cards = {}, effect_context ctx = {}) {
        if (!is_response && (origin_card->pocket == pocket_type::player_hand || origin_card->pocket == pocket_type::shop_selection) && !origin_card->is_brown()) {
            play_card_args ret { .card = origin_card };
            if (!origin_card->self_equippable()) {
                ret.targets.emplace_back(enums::enum_tag<target_type::player>,
                    random_element(make_equip_set(origin, origin_card), origin->m_game->rng));
            }
            return ret;
        } else if (origin_card->is_modifier()) {
            modifier_cards.push_back(origin_card);
            origin_card->modifier.add_context(origin_card, origin, ctx);
            origin_card = random_card_playable_with_modifiers(origin, is_response, modifier_cards, ctx);
            return generate_random_play(origin, origin_card, is_response, std::move(modifier_cards), ctx);
        } else {
            play_card_args ret { .card = origin_card, .is_response = is_response };
            for (card *mod_card : modifier_cards) {
                ret.modifiers.emplace_back(mod_card, mod_card->get_effect_list(is_response)
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
            for (const effect_holder &holder : origin_card->get_effect_list(is_response)) {
                ret.targets.push_back(generate_random_target(origin, origin_card, holder, ctx));
            }
            if (is_possible_to_play_effects(origin, origin_card, origin_card->optionals, ctx)) {
                for (const effect_holder &holder : origin_card->optionals) {
                    ret.targets.push_back(generate_random_target(origin, origin_card, holder, ctx));
                }
            }
            return ret;
        }
    }

    static bool execute_random_play(player *origin, bool is_response, std::set<card *> const& cards, std::initializer_list<pocket_type> pockets) {
        for (int i=0; i<10; ++i) {
            std::set<card *> card_set = cards;
            while (!card_set.empty()) {
                card *selected_card = [&]{
                    for (pocket_type pocket : pockets) {
                        if (auto filter = card_set | std::views::filter([&](card *c) { return c->pocket == pocket; })) {
                            return random_element(filter, origin->m_game->rng);
                        }
                    }
                    return random_element(card_set, origin->m_game->rng);
                }();

                card_set.erase(selected_card);
                play_card_args args = generate_random_play(origin, selected_card, is_response);

                if (!verify_and_play(origin, args.card, args.is_response, args.targets, args.modifiers)) {
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
            get_all_active_cards(origin)
            | ranges::views::filter([&](card *target_card){
                return is_possible_to_play(origin, target_card);
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