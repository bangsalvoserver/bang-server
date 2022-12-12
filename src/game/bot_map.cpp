#include "bot_map.h"

#include "game.h"
#include "play_verify.h"

namespace banggame {

    struct bot_impl {
        player *origin;

        bot_impl(player *origin) : origin(origin) {}

        void request_play();

        void respond_to_request();

        void play_in_turn();
    };

    bot::bot(player *origin) : m_pimpl(std::make_unique<bot_impl>(origin)) {}

    bot::~bot() = default;

    void bot::request_play() {
        m_pimpl->request_play();
    }

    void bot_impl::request_play() {
        if (origin->m_game->pending_requests()) {
            respond_to_request();
        } else if (!origin->m_game->locked() && origin->m_game->m_playing == origin) {
            play_in_turn();
        }
    }

    decltype(auto) random_element(const auto &range, auto &rng) {
        return range[std::uniform_int_distribution{size_t(0), range.size() - 1}(rng)];
    }

    decltype(auto) random_element_prefer_not_last(const auto &range, auto &rng) {
        if (range.size() == 1) {
            return range[0];
        } else {
            return range[std::uniform_int_distribution{size_t(0), range.size() - 2}(rng)];
        }
    }

    void bot_impl::respond_to_request() {
        auto update = origin->m_game->make_request_update(origin);

        if (!update.respond_cards.empty()) {
            while (true) {
                play_card_verify verifier{origin, random_element_prefer_not_last(update.respond_cards, origin->m_game->rng), true};

                for (const effect_holder &holder : verifier.origin_card->responses) {
                    switch (holder.target) {
                    case target_type::none:
                        verifier.targets.emplace_back(enums::enum_tag<target_type::none>);
                        break;
                    case target_type::player:
                        verifier.targets.emplace_back(enums::enum_tag<target_type::player>,
                            random_element(origin->make_player_target_set(verifier.origin_card, holder), origin->m_game->rng));
                        break;
                    case target_type::conditional_player:
                        verifier.targets.emplace_back(enums::enum_tag<target_type::conditional_player>,
                            random_element(origin->make_player_target_set(verifier.origin_card, holder), origin->m_game->rng));
                        break;
                    case target_type::card:
                        verifier.targets.emplace_back(enums::enum_tag<target_type::card>,
                            random_element(origin->make_card_target_set(verifier.origin_card, holder), origin->m_game->rng));
                    case target_type::extra_card:
                        verifier.targets.emplace_back(enums::enum_tag<target_type::extra_card>,
                            random_element(origin->make_card_target_set(verifier.origin_card, holder), origin->m_game->rng));
                    case target_type::players:
                        verifier.targets.emplace_back(enums::enum_tag<target_type::players>);
                        break;
                    case target_type::cards: {
                        std::vector<card *> out_cards;
                        std::ranges::sample(origin->make_card_target_set(verifier.origin_card, holder),
                            std::back_inserter(out_cards), holder.target_value, origin->m_game->rng);
                        verifier.targets.emplace_back(enums::enum_tag<target_type::cards>, to_vector_not_null(out_cards));
                        break;
                    }
                    case target_type::cards_other_players:
                        verifier.targets.emplace_back(enums::enum_tag<target_type::cards_other_players>); // TODO
                        break;
                    case target_type::select_cubes:
                        verifier.targets.emplace_back(enums::enum_tag<target_type::select_cubes>); // TODO
                        break;
                    case target_type::self_cubes:
                        verifier.targets.emplace_back(enums::enum_tag<target_type::self_cubes>);
                        break;
                    }
                }

                if (auto error = verifier.verify_and_play()) {
                    std::cout << "bot error " << error.format_str << "\n";
                } else {
                    break;
                }
            }
        } else if (!update.pick_cards.empty()) {
            origin->m_game->top_request().on_pick(random_element(update.pick_cards, origin->m_game->rng));
        }
    }

    void bot_impl::play_in_turn() {
        origin->pass_turn();
    }

}