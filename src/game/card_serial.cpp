#include "card_serial.h"
#include "game.h"

namespace json {

template<> Json::Value serializer<banggame::card *, banggame::game_table>::operator()(banggame::card *card) const {
    if (card) {
        return card->id;
    } else {
        return Json::nullValue;
    }
}

template<> Json::Value serializer<banggame::player *, banggame::game_table>::operator()(banggame::player *player) const {
    if (player) {
        return player->id;
    } else {
        return Json::nullValue;
    }
}

template<> banggame::card *deserializer<banggame::card *, banggame::game_table>::operator()(const Json::Value &value) const {
    if (value.isInt()) {
        return context.find_card(value.asInt());
    } else {
        return nullptr;
    }
}

template<> banggame::player *deserializer<banggame::player *, banggame::game_table>::operator()(const Json::Value &value) const {
    if (value.isInt()) {
        return context.find_player(value.asInt());
    } else {
        return nullptr;
    }
}

}