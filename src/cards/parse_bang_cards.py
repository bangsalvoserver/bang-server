#!/usr/bin/env python3

import re
import sys
import yaml
from cpp_generator import CppEnum, print_cpp_file

def is_hidden(card):
    return 'tags' in card and any(value == 'hidden' for value in card['tags'])

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

        if player_filter and target_type not in ('player', 'conditional_player', 'players', 'card', 'extra_card', 'cards'):
            raise RuntimeError(f'Invalid effect string: {effect}\nPlayer filter not allowed with {target_type}')
            
        if card_filter and target_type not in ('card', 'extra_card', 'cards'):
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

def parse_effect_simple(effect_list, value_name, type_name):
    if not isinstance(effect_list, list):
        raise RuntimeError(f'in parse_effect_simple: expected list, got {effect_list}')

    result = []
    for effect in effect_list:
        match = re.match(
            r'^\s*(\w+)' # type
            r'(?:\s*\((-?\d+)\))?\s*$', # effect_value
            effect
        )
        if not match:
            raise RuntimeError(f'Invalid effect string: {effect}')
        
        effect_type = match.group(1)
        effect_value = match.group(2)

        result.append({
            value_name: int(effect_value) if effect_value else None,
            'type': CppEnum(type_name, effect_type)
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
            'equips':       parse_effect_simple(card['equip'], 'effect_value', 'equip_type') if 'equip' in card else None,
            'tags':         parse_effect_simple(card['tags'], 'tag_value', 'tag_type') if 'tags' in card else None,
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

def add_expansion(card, name):
    if 'expansion' in card:
        card['expansion'] += ' ' + name
    else:
        card['expansion'] = name

def parse_file(data):
    result = {
        'deck': [],
        'characters': [],
        'goldrush': [],
        'highnoon': [],
        'fistfulofcards': [],
        'wildwestshow': [],
        'button_row': [],
        'stations': [],
        'train': [],
        'locomotive': [],
        'hidden': []
    }

    for card in data['main_deck']:
        card['deck'] = 'main_deck'
        if is_hidden(card):
            result['hidden'].append(parse_all_effects(card))
        else:
            for sign in card['signs']:
                card['sign'] = sign
                result['deck'].append(parse_all_effects(card))
    
    for card in data['character']:
        card['deck'] = 'character'
        result['characters'].append(parse_all_effects(card))
    
    for card in data['goldrush']:
        card['deck'] = 'goldrush'
        add_expansion(card, 'goldrush')
        count = card['count'] if 'count' in card else 1
        for _ in range(count):
            if is_hidden(card):
                result['hidden'].append(parse_all_effects(card))
            else:
                result['goldrush'].append(parse_all_effects(card))

    for name in ('highnoon', 'fistfulofcards', 'wildwestshow'):
        for card in data[name]:
            card['deck'] = name
            add_expansion(card, name)
            if is_hidden(card):
                result['hidden'].append(parse_all_effects(card))
            else:
                result[name].append(parse_all_effects(card))

    for card in data['station']:
        card['deck'] = 'station'
        add_expansion(card, 'greattrainrobbery')
        result['stations'].append(parse_all_effects(card))

    for card in data['train']:
        card['deck'] = 'train'
        add_expansion(card, 'greattrainrobbery')
        if is_hidden(card):
            result['hidden'].append(parse_all_effects(card))
        else:
            result['train'].append(parse_all_effects(card))
    
    for card in data['locomotive']:
        card['deck'] = 'locomotive'
        add_expansion(card, 'greattrainrobbery')
        result['locomotive'] .append(parse_all_effects(card))

    for card in data['button_row']:
        if is_hidden(card):
            result['hidden'].append(parse_all_effects(card))
        else:
            result['button_row'].append(parse_all_effects(card))

    return result

INCLUDE_FILENAMES = ['cards/card_data.h', 'cards/effect_enums.h']
OBJECT_DECLARATION = 'all_cards_t banggame::all_cards'

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print(f'Usage: {sys.argv[0]} bang_cards.yml bang_cards.cpp')
        sys.exit(1)

    with open(sys.argv[1], 'r', encoding='utf8') as file:
        bang_cards = parse_file(yaml.safe_load(file))
    
    with open(sys.argv[2], 'w', encoding='utf8') as file:
        print_cpp_file(bang_cards, OBJECT_DECLARATION,
            include_filenames=INCLUDE_FILENAMES,
            file=file)
