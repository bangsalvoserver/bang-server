#include "card_serial.h"
#include "game.h"

namespace json {

template<> Json::Value serializer<banggame::card *, banggame::game>::operator()(banggame::card *card) const {
    if (card) {
        return card->id;
    } else {
        return Json::nullValue;
    }
}

template<> Json::Value serializer<banggame::player *, banggame::game>::operator()(banggame::player *player) const {
    if (player) {
        return player->id;
    } else {
        return Json::nullValue;
    }
}

template<> banggame::card *deserializer<banggame::card *, banggame::game>::operator()(const Json::Value &value) const {
    if (value.isNull()) {
        return nullptr;
    } else {
        return context.find_card(value.asInt());
    }
}

template<> banggame::player *deserializer<banggame::player *, banggame::game>::operator()(const Json::Value &value) const {
    if (value.isNull()) {
        return nullptr;
    } else {
        return context.find_player(value.asInt());
    }
}

}