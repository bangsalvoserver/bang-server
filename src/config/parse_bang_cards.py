#!/usr/bin/env python3

import re
import sys
import yaml_custom as yaml
from cpp_generator import CppEnum, print_cpp_file

def parse_sign(sign):
    match = re.match(r'^\s*([\w\d]+)\s*(\w+)\s*$', sign)
    if match:
        return {
            'suit': CppEnum('card_suit', match.group(2)),
            'rank' : CppEnum('card_rank', 'rank_' + match.group(1))
        }
    else:
        raise RuntimeError(f'Invalid sign string: {sign}')

def parse_effects(effect_list):
    if not isinstance(effect_list, list):
        raise RuntimeError(f'in parse_effects: expected list, got {effect_list}')

    result = []
    for effect in effect_list:
        match = re.match(
            r'^\s*(\w+)' # effect_type
            r'(?:\s*\((-?\d+)\))?' # effect_value
            r'(?:\s*(\w+)' # target_type
            r'\s*(?:\((-?\d+)\))?)?' # target_value
            r'([\w\s]*?)' # player_filter
            r'(?:\s*\|\s*([\w\s]+))?\s*$', # card_filter
            effect
        )
        if not match:
            raise RuntimeError(f'Invalid effect string: {effect}')

        effect_type = match.group(1)
        effect_value = match.group(2)
        target_type = match.group(3)
        target_value = match.group(4)
        player_filter = match.group(5)
        card_filter = match.group(6)

        if player_filter and target_type not in ('player', 'conditional_player', 'players', 'card', 'extra_card', 'cards', 'max_cards'):
            raise RuntimeError(f'Invalid effect string: {effect}\nPlayer filter not allowed with {target_type}')
            
        if card_filter and target_type not in ('card', 'extra_card', 'cards', 'max_cards'):
            raise RuntimeError(f'Invalid effect string: {effect}\nCard filter not allowed with {target_type}')

        result.append({
            'target':           CppEnum('target_type', target_type) if target_type else None,
            'player_filter':    CppEnum('target_player_filter', player_filter) if player_filter else None,
            'card_filter':      CppEnum('target_card_filter', card_filter) if card_filter else None,
            'effect_value':     int(effect_value) if effect_value else None,
            'target_value':     int(target_value) if target_value else None,
            'type':             CppEnum('effect_type', effect_type)
        })
    return result

def parse_equips(equip_list):
    if not isinstance(equip_list, list):
        raise RuntimeError(f'in parse_equip: expected list, got {equip_list}')

    result = []
    for effect in equip_list:
        match = re.match(
            r'^\s*(\w+)' # type
            r'(?:\s*\((-?\d+)\))?\s*$', # effect_value
            effect
        )
        if not match:
            raise RuntimeError(f'Invalid equip string: {effect}')
        
        effect_type = match.group(1)
        effect_value = match.group(2)

        result.append({
            'effect_value': int(effect_value) if effect_value else None,
            'type': CppEnum('equip_type', effect_type)
        })
    return result

def parse_tags(tag_list):
    if not isinstance(tag_list, list):
        raise RuntimeError(f'in parse_tags: expected list, got {tag_list}')

    result = []
    for effect in tag_list:
        match = re.match(
            r'^\s*(\w+)' # type
            r'(?:\s*\((-?\d+)\))?\s*$', # tag_value
            effect
        )
        if not match:
            raise RuntimeError(f'Invalid tag string: {effect}')
        
        effect_type = match.group(1)
        effect_value = match.group(2)

        result.append({
            'tag_value': int(effect_value) if effect_value else None,
            'type': CppEnum('tag_type', effect_type)
        })
    return result

def parse_all_effects(card):
    try:
        return {
            'name':         card['name'] if 'name' in card else None,
            'image':        card['image'] if 'image' in card else None,
            'effects':      parse_effects(card['effects']) if 'effects' in card else None,
            'responses':    parse_effects(card['responses']) if 'responses' in card else None,
            'optionals':    parse_effects(card['optional']) if 'optional' in card else None,
            'equips':       parse_equips(card['equip']) if 'equip' in card else None,
            'tags':         parse_tags(card['tags']) if 'tags' in card else None,
            'expansion':    CppEnum('expansion_type', card['expansion']) if 'expansion' in card else None,
            'deck':         CppEnum('card_deck_type', card['deck']) if 'deck' in card else None,
            'modifier':     {'type': CppEnum('modifier_type', card['modifier'])} if 'modifier' in card else None,
            'mth_effect':   {'type': CppEnum('mth_type', card['mth_effect'])} if 'mth_effect' in card else None,
            'mth_response': {'type': CppEnum('mth_type', card['mth_response'])} if 'mth_response' in card else None,
            'equip_target': CppEnum('target_player_filter', card['equip_target']) if 'equip_target' in card else None,
            'color':        CppEnum('card_color_type', card['color']) if 'color' in card else None,
            'sign':         parse_sign(card['sign']) if 'sign' in card else None
        }
    except RuntimeError as error:
        raise RuntimeError(f"Error in card {card['name']}:\n{error}") from error

def merge_cards(card_sets):
    result = {}
    if not isinstance(card_sets, dict):
        raise RuntimeError(f'Error in merge_cards: Expected dict, got {card_sets}')
    for expansion, card_set in card_sets.items():
        for deck, cards in card_set.items():
            if not isinstance(cards, list):
                raise RuntimeError(f'Error in merge_cards: Expected list, got {cards}')
            if expansion != 'base':
                for card in cards:
                    if 'expansion' in card:
                        card['expansion'] += ' ' + expansion
                    else:
                        card['expansion'] = expansion
            if deck in result:
                result[deck].extend(cards)
            else:
                result[deck] = cards
    return result

def parse_file(data):
    def get_main_deck_cards(card):
        for sign in card['signs']:
            card['sign'] = sign
            yield card

    def get_goldrush_cards(card):
        return [card] * card.get('count', 1)
    
    def get_cards_default(card):
        return [card]

    class Deck:
        def __init__(self, strategy=get_cards_default, key=None, order=0, deck=None) -> None:
            self.strategy = strategy
            self.key = key
            self.order = order
            self.deck = deck

    DECKS = {
        'main_deck': Deck(strategy=get_main_deck_cards, key='deck', order=0),
        'character': Deck(key='characters', order=1),
        'goldrush': Deck(strategy=get_goldrush_cards, order=2),
        'highnoon': Deck(order=3),
        'fistfulofcards': Deck(order=4),
        'wildwestshow': Deck(order=5),
        'button_row': Deck(deck='none', order=6),
        'station': Deck(key='stations', order=7),
        'train': Deck(order=8),
        'locomotive': Deck(order=9),
        'hidden': Deck(deck='none', order=10),
    }

    def get_cards_for_deck(key, cards):
        deck = DECKS.get(key, Deck())
        def add_deck(card):
            if 'deck' not in card:
                card['deck'] = deck.deck or key
            return card
        return deck.key or key, list(parse_all_effects(card) for c in cards for card in deck.strategy(add_deck(c)))

    return dict(get_cards_for_deck(*item) for item in sorted(data.items(), key=lambda item: DECKS.get(item[0], Deck()).order))

INCLUDE_FILENAMES = ['cards/card_data.h', 'cards/effect_enums.h']
OBJECT_DECLARATION = 'all_cards_t banggame::all_cards'

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print(f'Usage: {sys.argv[0]} bang_cards.yml bang_cards.cpp')
        sys.exit(1)

    with open(sys.argv[1], 'r', encoding='utf8') as file:
        bang_cards = parse_file(merge_cards(yaml.safe_load(file)))
    
    if sys.argv[2] == '-':
        print_cpp_file(bang_cards, OBJECT_DECLARATION,
            include_filenames=INCLUDE_FILENAMES,
            file=sys.stdout)
    else:
        with open(sys.argv[2], 'w', encoding='utf8') as file:
            print_cpp_file(bang_cards, OBJECT_DECLARATION,
                include_filenames=INCLUDE_FILENAMES,
                file=file)
