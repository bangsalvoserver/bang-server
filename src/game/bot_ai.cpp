#include "game.h"
#include "play_verify.h"

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

    static play_card_verify generate_random_play(player *origin, card *origin_card, bool is_response) {
        play_card_verify verifier{origin, origin_card, is_response};

        if (!is_response && (origin_card->pocket == pocket_type::player_hand || origin_card->pocket == pocket_type::shop_selection) && !origin_card->is_brown()) {
            if (!origin_card->self_equippable()) {
                verifier.targets.emplace_back(enums::enum_tag<target_type::player>,
                    random_element(origin->make_equip_set(origin_card), origin->m_game->rng));
            }
        } else {
            auto &effects = is_response ? origin_card->responses : origin_card->effects;

            for (const effect_holder &holder : effects) {
                verifier.targets.push_back([&]() -> play_card_target {
                    switch (holder.target) {
                    case target_type::none:
                        return enums::enum_tag<target_type::none>;
                    case target_type::player:
                        return {enums::enum_tag<target_type::player>,
                            random_element(origin->make_player_target_set(origin_card, holder), origin->m_game->rng)};
                    case target_type::conditional_player:
                        return {enums::enum_tag<target_type::conditional_player>,
                            random_element(origin->make_player_target_set(origin_card, holder), origin->m_game->rng)};
                    case target_type::card:
                        return {enums::enum_tag<target_type::card>,
                            random_element(origin->make_card_target_set(origin_card, holder), origin->m_game->rng)};
                    case target_type::extra_card:
                        return {enums::enum_tag<target_type::extra_card>,
                            random_element(origin->make_card_target_set(origin_card, holder), origin->m_game->rng)};
                    case target_type::players:
                        return enums::enum_tag<target_type::players>;
                    case target_type::cards:
                        return {enums::enum_tag<target_type::cards>, [&]{
                            std::vector<not_null<card *>> ret;
                            std::ranges::sample(origin->make_card_target_set(origin_card, holder),
                                std::back_inserter(ret), holder.target_value, origin->m_game->rng);
                            return ret;
                        }()};
                    case target_type::cards_other_players:
                        return {enums::enum_tag<target_type::cards_other_players>, [&]{
                            std::vector<not_null<card *>> ret;
                            for (player *target : range_other_players(origin)) {
                                if (auto cards = util::concat_view(
                                    target->m_table | std::views::filter(std::not_fn(&card::is_black)),
                                    target->m_hand | std::views::take(1)
                                )) {
                                    ret.emplace_back(random_element(cards, origin->m_game->rng));
                                }
                            }
                            return ret;
                        }()};
                    case target_type::select_cubes:
                        return {enums::enum_tag<target_type::select_cubes>, [&]{
                            std::vector<card *> cubes;
                            for (card *slot : origin->cube_slots()) {
                                for (int i=0; i<slot->num_cubes; ++i) {
                                    cubes.push_back(slot);
                                }
                            }
                            
                            std::vector<not_null<card *>> ret;
                            std::ranges::sample(cubes, std::back_inserter(ret), holder.target_value, origin->m_game->rng);
                            return ret;
                        }()};
                    case target_type::self_cubes:
                        return enums::enum_tag<target_type::self_cubes>;
                    default:
                        throw std::runtime_error("Invalid target_type");
                    }
                }());
            }
        }

        return verifier;
    }

    static void respond_to_request(player *origin) {
        auto update = origin->m_game->make_request_update(origin);

        if (!update.pick_cards.empty() && std::ranges::all_of(update.respond_cards, [](card *c) { return c->pocket == pocket_type::button_row; })) {
            origin->m_game->top_request().on_pick(random_element(update.pick_cards, origin->m_game->rng));
        } else if (!update.respond_cards.empty()) {
            std::set<card *> cards{update.respond_cards.begin(), update.respond_cards.end()};
            while (!cards.empty()) {
                card *origin_card = random_card_of_pocket(update.respond_cards, origin->m_game->rng,
                    pocket_type::player_character, pocket_type::player_table, pocket_type::player_hand);
                if (!origin_card) break;

                cards.erase(origin_card);
                auto verifier = generate_random_play(origin, origin_card, true);

                if (!verifier.verify_and_play()) {
                    if (auto &prompt = origin->m_prompt) {
                        auto lock = origin->m_game->lock_updates();
                        auto fun = std::move(prompt->first);
                        prompt.reset();
                        std::invoke(fun);
                    }
                    return;
                }
            }
            // softlock
            std::cout << "BOT ERROR: could not find response card\n";
        } 
    }

    static void play_in_turn(player *origin) {
        auto cards_view = util::concat_view(
            std::views::all(origin->m_characters),
            std::views::all(origin->m_table) | std::views::filter(std::not_fn(&card::inactive)),
            std::views::all(origin->m_hand),
            std::views::all(origin->m_game->m_shop_selection) | std::views::filter([origin](card *target_card) {
                return target_card->buy_cost() <= origin->m_gold;
            }),
            std::views::all(origin->m_game->m_button_row))
            | std::views::filter([&](card *target_card){
                if (target_card->pocket != pocket_type::player_hand && target_card->pocket != pocket_type::shop_selection || target_card->is_brown()) {
                    return target_card->modifier == card_modifier_type::none
                        && origin->is_possible_to_play(target_card, false);
                } else {
                    return !origin->make_equip_set(target_card).empty();
                }
            });

        auto cards = std::set<card *>(std::ranges::begin(cards_view), std::ranges::end(cards_view));
        
        while (!cards.empty()) {
            card *origin_card = random_card_of_pocket(cards, origin->m_game->rng,
                pocket_type::player_character, pocket_type::player_table, pocket_type::player_hand);
            if (!origin_card) break;

            cards.erase(origin_card);
            auto verifier = generate_random_play(origin, origin_card, false);

            if (!verifier.verify_and_play()) {
                if (auto &prompt = origin->m_prompt) {
                    prompt.reset();
                } else {
                    return;
                }
            }
        }

        origin->pass_turn();
    }
    
    struct bot_delay_request : request_base {
        bot_delay_request(player *target) : request_base(nullptr, nullptr, target) {}

        struct bot_delay_timer : request_timer {
            using request_timer::request_timer;

            void on_finished() override {
                play_in_turn(request->target);
            }
        };
        
        bot_delay_timer m_timer{this, 0ms};
        request_timer *timer() override { return &m_timer; }
    };

    void game::request_bot_play(player *origin, bool is_response) {
        if (origin->is_bot() && !check_flags(game_flags::game_over)) {
            if (is_response) {
                respond_to_request(origin);
            } else {
                queue_request_front<bot_delay_request>(origin);
            }
        }
    }
}