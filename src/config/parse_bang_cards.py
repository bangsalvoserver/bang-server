#!/usr/bin/env python3

import re
import sys
import yaml_custom as yaml
from cpp_generator import *

def parse_sign(sign):
    match = re.match(r'^\s*([\w\d]+)\s*(\w+)\s*$', sign)
    if match:
        return CppObject(
            suit = CppEnum('card_suit', match.group(2)),
            rank = CppEnum('card_rank', 'rank_' + match.group(1))
        )
    else:
        raise RuntimeError(f'Invalid sign string: {sign}')
    
def check_allowed_filters(effect, target_type, player_filter, card_filter):
    NONE = 0
    PLAYER = 1
    CARD = 2

    ALLOWED_FILTERS = {
        'none': NONE,
        'player': PLAYER,
        'conditional_player': PLAYER,
        'adjacent_players': PLAYER,
        'player_per_cube': PLAYER,
        'card': CARD,
        'random_if_hand_card': CARD,
        'extra_card': CARD,
        'players': PLAYER,
        'cards': CARD,
        'max_cards': CARD,
        'card_per_player': CARD,
        'cube_slot': PLAYER,
        'move_cube_slot': NONE,
        'select_cubes': NONE,
        'select_cubes_optional': NONE,
        'select_cubes_repeat': NONE,
        'self_cubes': NONE
    }

    if target_type and target_type not in ALLOWED_FILTERS:
        raise RuntimeError(f'Invalid target_type: {target_type}')
    
    allowed_type = ALLOWED_FILTERS[target_type] if target_type else NONE

    if player_filter and allowed_type == NONE:
        raise RuntimeError(f'Invalid effect string: {effect}\nPlayer filter not allowed with {target_type}')
        
    if card_filter and allowed_type != CARD:
        raise RuntimeError(f'Invalid effect string: {effect}\nCard filter not allowed with {target_type}')

def parse_effects(effect_list):
    if not isinstance(effect_list, list):
        raise RuntimeError(f'in parse_effects: expected list, got {effect_list}')

    result = []
    for effect in effect_list:
        match = re.match(
            r'^\s*(\w+)' # effect_type
            r'(?:\s*\(\s*(.+?)\s*\))?' # effect_value
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

        check_allowed_filters(effect, target_type, player_filter, card_filter)

        result.append(CppObject(
            target =           CppLiteral(f'TARGET_TYPE({target_type})') if target_type else None,
            player_filter =    [CppEnum('target_player_filter', f) for f in player_filter.split()] if player_filter else None,
            card_filter =      [CppEnum('target_card_filter', f) for f in card_filter.split()] if card_filter else None,
            target_value =     int(target_value) if target_value else None,
            type =             CppLiteral(f'GET_EFFECT({effect_type})'),
            effect_value =     CppStatic('auto', CppLiteral(f"BUILD_EFFECT_VALUE({effect_type}{', ' + effect_value if effect_value else ''})"), pointer=True),
        ))
    return CppStatic('effect_holder', result)

def parse_equips(equip_list):
    if not isinstance(equip_list, list):
        raise RuntimeError(f'in parse_equip: expected list, got {equip_list}')

    result = []
    for effect in equip_list:
        match = re.match(
            r'^\s*(\w+)' # type
            r'(?:\s*\(\s*(.+?)\s*\))?\s*$', # effect_value
            effect
        )
        if not match:
            raise RuntimeError(f'Invalid equip string: {effect}')
        
        effect_type = match.group(1)
        effect_value = match.group(2)

        result.append(CppObject(
            type = CppLiteral(f'GET_EQUIP({effect_type})'),
            effect_value = CppStatic('auto', CppLiteral(f"BUILD_EQUIP_VALUE({effect_type}{', ' + effect_value if effect_value else ''})"), pointer=True),
        ))
    return CppStatic('equip_holder', result)

def parse_tags(tag_list):
    if not isinstance(tag_list, list):
        raise RuntimeError(f'in parse_tags: expected list, got {tag_list}')

    result = {}
    for tag in tag_list:
        match = re.match(
            r'^\s*(\w+)' # type
            r'(?:\s*\((-?\d+)\))?\s*$', # tag_value
            tag
        )
        if not match:
            raise RuntimeError(f'Invalid tag string: {tag}')
        
        tag_type_str = match.group(1)
        tag_value = match.group(2)

        tag_type = CppEnum('tag_type', tag_type_str)
        if tag_type in result:
            raise RuntimeError(f'Duplicate tag: {tag_type_str}')
        result[tag_type] = int(tag_value) if tag_value else 0
    return CppStaticMap('tag_type', 'short', result)

def parse_modifier(modifier):
    match = re.match(
        r'^\s*(\w+)' # type
        r'(?:\s*\(\s*(.+?)\s*\))?\s*$', # effect_value
        modifier
    )
    if not match:
        raise RuntimeError(f'Invalid modifier string: {modifier}')
    
    effect_type = match.group(1)
    effect_value = match.group(2)
    return CppObject(
        type = CppLiteral(f"GET_MODIFIER({modifier})"),
        effect_value = CppStatic('auto', CppLiteral(f"BUILD_MODIFIER_VALUE({effect_type}{', ' + effect_value if effect_value else ''})"), pointer=True),
    )

def parse_mth(effect):
    match = re.match(
        r'^(\w+)\s*' # type
        r'\(((?:\s*\d+,)*\s*\d+\s*)\)' # indices
        r'(?:\s*\(\s*(.+?)\s*\))?\s*$', # effect_value
        effect
    )
    if not match:
        raise RuntimeError(f'Invalid mth string: {effect}')
    
    effect_type = match.group(1)
    indices = match.group(2)
    effect_value = match.group(3)
    return CppObject(
        type = CppLiteral(f'GET_MTH({effect_type})'),
        effect_value = CppStatic('auto', CppLiteral(
            f"BUILD_MTH_VALUE({effect_type}, "
            f"({', '.join(value.strip() for value in indices.split(','))})"
            + ((', ' + effect_value) if effect_value else '')
            + ')'
        ), pointer=True),
    )

def parse_expansions(expansions, fn = list):
    return CppStatic('ruleset_ptr', fn(CppLiteral(f"GET_RULESET({expansion})") for expansion in expansions ))

def parse_all_effects(card):
    try:
        return CppObject(
            name =         card['name'] if 'name' in card else None,
            image =        card['image'] if 'image' in card else None,
            effects =      parse_effects(card['effects']) if 'effects' in card else None,
            responses =    parse_effects(card['responses']) if 'responses' in card else None,
            equips =       parse_equips(card['equip']) if 'equip' in card else None,
            tags =         parse_tags(card['tags']) if 'tags' in card else None,
            expansion =    parse_expansions(card['expansion'].split(), set) if 'expansion' in card else None,
            deck =         CppEnum('card_deck_type', card['deck']) if 'deck' in card else None,
            modifier =      parse_modifier(card['modifier']) if 'modifier' in card else None,
            modifier_response = parse_modifier(card['modifier_response']) if 'modifier_response' in card else None,
            mth_effect =   parse_mth(card['mth_effect']) if 'mth_effect' in card else None,
            mth_response = parse_mth(card['mth_response']) if 'mth_response' in card else None,
            equip_target = [CppEnum('target_player_filter', f) for f in card['equip_target'].split()] if 'equip_target' in card else None,
            color =        CppEnum('card_color_type', card['color']) if 'color' in card else None,
            sign =         parse_sign(card['sign']) if 'sign' in card else None
        )
    except RuntimeError as error:
        raise RuntimeError(f"Error in card {card['name']}:\n{error}") from error

def merge_cards(card_sets):
    result = {}
    expansions = []
    if not isinstance(card_sets, dict):
        raise RuntimeError(f'Error in merge_cards: Expected dict, got {card_sets}')
    for expansion, card_set in card_sets.items():
        if expansion != 'base':
            expansions.append(expansion)
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
    return result, expansions

def parse_file(card_sets):
    def get_main_deck_cards(card):
        return [{'sign': sign, **card} for sign in card['signs']]

    def get_goldrush_cards(card):
        return [card] * card.get('count', 1)
    
    def get_cards_default(card):
        return [card]

    class Deck:
        def __init__(self, name, strategy=get_cards_default, key=None, deck=None) -> None:
            self.name = name
            self.strategy = strategy
            self.key = key
            self.deck = deck

        def get_cards(self, card_data):
            if 'deck' not in card_data:
                card_data['deck'] = self.deck or self.key or self.name
            return self.strategy(card_data)
        
    DECKS = [
        Deck('deck', strategy=get_main_deck_cards, key='main_deck'),
        Deck('characters', key='character'),
        Deck('goldrush', strategy=get_goldrush_cards),
        Deck('highnoon'),
        Deck('fistfulofcards'),
        Deck('wildwestshow'),
        Deck('button_row', deck='none'),
        Deck('stations', key='station'),
        Deck('train'),
        Deck('locomotive'),
        Deck('legends'),
        Deck('feats'),
        Deck('hidden', deck='none'),
    ]
    
    data, expansions = merge_cards(card_sets)

    return CppObject(**{
        deck.name : CppStatic('card_data', [
            parse_all_effects(card)
            for card_data in data[deck.key or deck.name]
            for card in deck.get_cards(card_data)
        ])
        for deck in DECKS
    }, expansions = parse_expansions(expansions))

INCLUDE_FILENAMES = ['cards/vtable_build.h', 'effects/effects.h']

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print(f'Usage: {sys.argv[0]} bang_cards.yml bang_cards.cpp')
        sys.exit(1)

    with open(sys.argv[1], 'r', encoding='utf8') as file:
        bang_cards = CppDeclaration(
            object_name='const all_cards_t all_cards',
            object_value=parse_file(yaml.safe_load(file)),
            namespace_name='banggame'
        )
    
    if sys.argv[2] == '-':
        print_cpp_file(bang_cards, include_filenames=INCLUDE_FILENAMES, file=sys.stdout)
    else:
        with open(sys.argv[2], 'w', encoding='utf8') as file:
            print_cpp_file(bang_cards, include_filenames=INCLUDE_FILENAMES, file=file)
