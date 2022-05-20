#!/usr/bin/env python

import sys
import yaml
import re

def parse_sign(sign):
    match = re.match(r"^\s*([\w\d]+)\s*(\w+)\s*", sign)
    if not match:
        raise ValueError(f'Invalid sign string: {sign}')
    return f'{{ card_suit::{match.group(2)}, card_rank::rank_{match.group(1)} }}'

def enum_flags(string, enum_name):
    return ' | '.join(f'{enum_name}::{name}' for name in string.split())

def parse_effects(effects_data, vec_name):
    print(f',\n      .{vec_name} = {{')
    last_elem = len(effects_data) - 1
    for i, effect in enumerate(effects_data):
        match = re.match(
            r"\s*(\w+)" # type
            r"(?:\s*\((-?\d+)\))?" # effect_value
            r"(?:\s*(\w+)\s*)?" # target
            r"([\w\s]*?)" # player_filter
            r"(?:\s*\|\s*([\w\s]+))?\s*$", #card_filter
            effect
        )
        if not match:
            raise ValueError(f'Invalid effect string: {effect}')
        print('        {')
        if match.group(3):
            print(f'          .target = play_card_target_type::{match.group(3)},')
        if match.group(4):
            if match.group(3) not in ('player','conditional_player','card'):
                raise ValueError(f'Invalid effect string: {effect}\nPlayer filter not allowed with {match.group(3)} target')
            print(f'          .player_filter = {enum_flags(match.group(4), "target_player_filter")},')
        if match.group(5):
            if match.group(3) != 'card':
                raise ValueError(f'Invalid effect string: {effect}\nCard filter not allowed with {match.group(3)} target')
            print(f'          .card_filter = {enum_flags(match.group(5), "target_card_filter")},')
        if match.group(2):
            print(f'          .effect_value = {match.group(2)},')
        print(f'          .type = effect_type::{match.group(1)}')
        if i == last_elem:
            print('        }')
        else:
            print('        },')
    print('      }', end='')

def parse_equips(equips_data):
    print(',\n      .equips = {')
    last_elem = len(equips_data) - 1
    for i, equip in enumerate(equips_data):
        match = re.match(
            r"^\s*(\w+)"
            r"(?:\s*\((-?\d+)\))?\s*$",
            equip
        )
        if not match:
            raise ValueError(f'Invalid equip string: {equip}')
        print('        {')
        if match.group(2):
            print(f'          .effect_value = {match.group(2)},')
        print(f'          .type = equip_type::{match.group(1)}')
        if i == last_elem:
            print('        }')
        else:
            print('        },')
    print('      }', end='')

def parse_tags(tags_data):
    print(',\n      .tags = {')
    last_elem = len(tags_data) - 1
    for i, tag in enumerate(tags_data):
        match = re.match(
            r"^\s*(\w+)"
            r"(?:\s*\((-?\d+)\))?\s*$",
            tag
        )
        if not match:
            raise ValueError(f'Invalid equip string: {tag}')
        print('        {')
        if match.group(2):
            print(f'          .tag_value = {match.group(2)},')
        print(f'          .type = tag_type::{match.group(1)}')
        if i == last_elem:
            print('        }')
        else:
            print('        },')
    print('      }', end='')

def parse_all_effects(card):
    print('    {')
    print(f'      .name = "{card["name"]}"', end='')
    try:
        if 'image' in card:
            print(f',\n      .image = "{card["image"]}"', end='')
        if 'effects' in card:
            parse_effects(card['effects'], 'effects')
        if 'responses' in card:
            parse_effects(card['responses'], 'responses')
        if 'optional' in card:
            parse_effects(card['optional'], 'optionals')
        if 'equip' in card:
            parse_equips(card['equip'])
        if 'tags' in card:
            parse_tags(card['tags'])
        if 'expansion' in card:
            print(f',\n      .expansion = {enum_flags(card["expansion"], "card_expansion_type")}', end='')
        if 'deck' in card:
            print(f',\n      .deck = card_deck_type::{card["deck"]}', end='')
        if 'modifier' in card:
            print(f',\n      .modifier = card_modifier_type::{card["modifier"]}', end='')
        if 'multitarget' in card:
            print(',\n      .multi_target_handler = {')
            print(f'        .type = mth_type::{card["multitarget"]}')
            print('      }', end='')
        if 'equip_target' in card:
            print(f',\n      .equip_target = target_player_filter::{card["equip_target"]}', end='')
        if 'color' in card:
            print(f',\n      .color = card_color_type::{card["color"]}', end='')
    except ValueError as error:
        print(f'Error in card {card["name"]}:\n{error}', file=sys.stderr)
        exit(1)

def parse_file(data):
    hidden_cards = []

    print('// AUTO GENERATED FILE')
    print('#include "game/card_data.h"\n')
    print('namespace banggame {\n')
    print('using namespace enums::flag_operators;\n')
    print('const all_cards_t all_cards = {\n  .deck = {')
    for card in data['main_deck']:
        if 'disabled' in card and card['disabled']:
            continue
        card['deck'] = 'main_deck'
        for sign in card['signs']:
            parse_all_effects(card)
            print(f',\n      .sign = {parse_sign(sign)}')
            print('    },')
    print('  },')

    print('  .characters = {')
    for card in data['character']:
        if 'disabled' in card and card['disabled']:
            continue
        card['deck'] = 'character'
        parse_all_effects(card)
        print('\n    },')
    print('  },')

    print('  .goldrush = {')
    for card in data['goldrush']:
        if 'disabled' in card and card['disabled']:
            continue
        card['expansion'] = 'goldrush'
        card['deck'] = 'goldrush'
        for _ in range(card['count'] if 'count' in card else 1):
            if 'tags' in card and 'hidden' in card['tags']:
                hidden_cards.append(card)
            else:
                parse_all_effects(card)
                print('\n    },')
    print('  },')

    print('  .highnoon = {')
    for card in data['highnoon']:
        if 'disabled' in card and card['disabled']:
            continue
        card['expansion'] = f'highnoon {card["expansion"]}' if 'expansion' in card else 'highnoon'
        card['deck'] = 'highnoon'
        if 'tags' in card and 'hidden' in card['tags']:
            hidden_cards.append(card)
        else:
            parse_all_effects(card)
            print('\n    },')
    print('  },')

    print('  .fistfulofcards = {')
    for card in data['fistfulofcards']:
        if 'disabled' in card and card['disabled']:
            continue
        card['expansion'] = f'fistfulofcards {card["expansion"]}' if 'expansion' in card else 'fistfulofcards'
        card['deck'] = 'fistfulofcards'
        if 'tags' in card and 'hidden' in card['tags']:
            hidden_cards.append(card)
        else:
            parse_all_effects(card)
            print('\n    },')
    print('  },')

    print('  .wildwestshow = {')
    for card in data['wildwestshow']:
        if 'disabled' in card and card['disabled']:
            continue
        card['expansion'] = 'wildwestshow'
        card['deck'] = 'wildwestshow'
        parse_all_effects(card)
        print('\n    },')
    print('  },')

    print('  .specials = {')
    for card in data['specials']:
        if 'disabled' in card and card['disabled']:
            continue
        if 'tags' in card and 'hidden' in card['tags']:
            hidden_cards.append(card)
        else:
            parse_all_effects(card)
            print('\n    },')
    print('  },')

    print('  .hidden = {')
    for card in hidden_cards:
        parse_all_effects(card)
        print('\n    },')
    print('  }\n};\n\n}')

if __name__ == '__main__':
    with open(sys.argv[1], 'r', encoding='utf8') as file:
        parse_file(yaml.safe_load(file))
    