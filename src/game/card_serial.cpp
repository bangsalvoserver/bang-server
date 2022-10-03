#include "card_serial.h"
#include "game.h"

namespace banggame::serial {
    card_id_vector::card_id_vector(std::span<banggame::card *> cards)
        : card_ids{to_vector(std::views::transform(cards, &banggame::card::id))} {}
}

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

template<> Json::Value serializer<banggame::serial::card_id_vector, banggame::game>::operator()(const banggame::serial::card_id_vector &value) const {
    return serialize(value.card_ids);
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