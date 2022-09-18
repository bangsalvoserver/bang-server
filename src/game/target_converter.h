#ifndef __TARGET_CONVERTER_H__
#define __TARGET_CONVERTER_H__

#include "game.h"
#include "player.h"

namespace banggame {
    template<typename T> struct target_converter;

    template<> struct target_converter<card *> {
        card *operator()(game *game, int id) {
            return game->find_card(id);
        }
    };

    template<> struct target_converter<player *> {
        player *operator()(game *game, int id) {
            return game->find_player(id);
        }
    };

    template<> struct target_converter<int> {
        int operator()(game *game, int value) {
            return value;
        }
    };

    template<typename T> struct target_converter<nullable<T>> {
        nullable<T> operator()(game *game, int id) {
            if (id) {
                return nullable<T>(target_converter<T *>{}(game, id));
            } else {
                return {};
            }
        }
    };

    template<typename T> struct target_converter<std::vector<T>> {
        template<typename U>
        std::vector<T> operator()(game *game, const std::vector<U> &args) {
            std::vector<T> ret;
            for (const U &arg : args) {
                ret.push_back(target_converter<T>{}(game, arg));
            }
            return ret;
        }
    };

    struct target_id_visitor {
        game *m_game;

        template<target_type E>
        play_card_target operator()(enums::enum_tag_t<E> tag) {
            return play_card_target(tag);
        }

        template<target_type E>
        play_card_target operator()(enums::enum_tag_t<E> tag, const auto &args) {
            return play_card_target(tag, target_converter<play_card_target::value_type<E>>{}(m_game, args));
        }
    };

    template<> struct target_converter<play_card_target> {
        play_card_target operator()(game *game, const play_card_target_id &args) {
            return enums::visit_indexed(target_id_visitor{game}, args);
        }
    };
}

#endif