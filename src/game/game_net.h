#ifndef __GAME_NET_H__
#define __GAME_NET_H__

#include <deque>
#include <iostream>

#include "player.h"
#include "format_str.h"
#include "game_update.h"

namespace banggame {

    inline std::vector<card_backface> make_id_vector(std::ranges::range auto &&range) {
        return to_vector(range | std::views::transform([](const card *c) {
            return card_backface{c->id, c->deck};
        }));
    }

    class update_target {
    private:
        player *m_targets[8];

        struct {
            bool m_inclusive:1;
            bool m_invert_public:1;
            int m_num_targets:6;
        };

        update_target(bool inclusive, bool invert_public, std::same_as<player *> auto ... targets)
            : m_targets{targets ...}
            , m_inclusive{inclusive}
            , m_invert_public{invert_public}
            , m_num_targets{sizeof...(targets)} {}

        update_target(bool inclusive, std::same_as<player *> auto ... targets)
            : update_target(inclusive, false, targets...) {}

    public:
        static update_target includes(std::same_as<player *> auto ... targets) {
            return update_target(true, targets...);
        }

        static update_target excludes(std::same_as<player *> auto ... targets) {
            return update_target(false, targets...);
        }

        static update_target includes_private(std::same_as<player *> auto ... targets) {
            return update_target(true, true, targets...);
        }

        static update_target excludes_public(std::same_as<player *> auto ... targets) {
            return update_target(false, true, targets...);
        }

        void add(player *target) {
            m_targets[m_num_targets++] = target;
        }

        bool matches(int user_id) const {
            std::span targets{m_targets, m_targets + m_num_targets};
            return (std::ranges::find(targets, user_id, &player::user_id) != targets.end()) == m_inclusive;
        }

        bool is_public() const {
            return m_invert_public != m_inclusive != (m_num_targets == 0);
        }
    };

    template<typename Context>
    struct game_net_manager {
        std::deque<std::pair<update_target, Json::Value>> m_updates;
        std::deque<std::pair<update_target, game_string>> m_saved_log;

        const Context &context() const {
            return static_cast<const Context &>(*this);
        }

        template<game_update_type E>
        Json::Value make_update(auto && ... args) {
            return json::serialize(game_update{enums::enum_tag<E>, FWD(args) ... }, context());
        }

        template<game_update_type E>
        void add_update(update_target target, auto && ... args) {
            m_updates.emplace_back(target, make_update<E>(FWD(args) ... ));
        }

        template<game_update_type E>
        void add_update(auto && ... args) {
            add_update<E>(update_target::excludes(), FWD(args) ... );
        }

        template<typename ... Ts>
        void add_log(update_target target, auto && ... args) {
            const auto &log = m_saved_log.emplace_back(std::piecewise_construct,
                std::make_tuple(target), std::make_tuple(FWD(args) ... ));
            add_update<game_update_type::game_log>(std::move(target), log.second);
        }

        void add_log(auto && ... args) {
            add_log(update_target::excludes(), FWD(args) ... );
        }
    };

}

#endif