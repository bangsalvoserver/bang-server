#!/usr/bin/env python3

from ast import parse
import enum
import yaml
import re
import sys

def set_expansion(card, value):
    if 'expansion' in card:
        card['expansion'] = '{} {}'.format(value, str(card['expansion']))
    else:
        card['expansion'] = value

def is_hidden(card):
    return 'tags' in card and any(value == 'hidden' for value in card['tags'])

def parse_sign(sign):
    match = re.match(r'^\s*([\w\d]+)\s*(\w+)\s*$', sign)
    if match:
        return 'card_suit::{}, card_rank::rank_{}'.format(match.group(2), match.group(1))
    else:
        raise RuntimeError(f'Invalid sign string: {sign}')

def enum_flags(signs, enum_name):
    return ' | '.join('{}::{}'.format(enum_name, sign) for sign in signs.split())

def parse_effects(out, list, name):
    print(',\n      .{} {{'.format(name), file=out)
    for i, effect in enumerate(list):
        match = re.match(
            r'^\s*(\w+)' # type
            r'(?:\s*\((-?\d+)\))?' # effect_value
            r'(?:\s*(\w+)' # target
            r'\s*(?:\((-?\d+)\))?)?' # target_value
            r'([\w\s]*?)' # player_filter
            r'(?:\s*\|\s*([\w\s]+))?\s*$', # card_filter
            effect
        )
        if not match:
            raise RuntimeError(f'Invalid effect string: {effect}')
        
        type = match.group(1)
        effect_value = match.group(2)
        target = match.group(3)
        target_value = match.group(4)
        player_filter = match.group(5)
        card_filter = match.group(6)

        print('        {', file=out)

        if target:
            print('          .target {{target_type::{}}},'.format(target), file=out)
        if player_filter:
            if target not in ('player', 'conditional_player', 'card'):
                raise RuntimeError('Invalid effect string: {0}\nPlayer filter not allowed with {1}'.format(effect, target))
            print('          .player_filter {{{}}},'.format(enum_flags(player_filter, 'target_player_filter')), file=out)
        if card_filter:
            if target != 'card':
                raise RuntimeError('Invalid effect string: {0}\nCard filter not alowed with {1}'.format(effect, target))
            print('          .card_filter {{{}}},'.format(enum_flags(card_filter, 'target_card_filter')), file=out)
        if effect_value:
            print('          .effect_value {{{}}},'.format(effect_value), file=out)
        if target_value:
            print('          .target_value {{{}}},'.format(target_value), file=out)
        print('          .type {{effect_type::{}}}\n        }}'.format(type), end='', file=out)
        if i == len(list)-1:
            print('', file=out)
        else:
            print(',', file=out)
    print('      }', end='', file=out)

def parse_effect_simple(out, list, name, value_name, type_name):
    print(',\n      .{} {{'.format(name), file=out)
    for i, effect in enumerate(list):
        match = re.match(
            r'^\s*(\w+)' # type
            r'(?:\s*\((-?\d+)\))?\s*$', # effect_value
            effect
        )
        if not match:
            raise RuntimeError(f'Invalid {name} string: {effect}')
        
        type = match.group(1)
        value = match.group(2)

        print('        {', file=out)
        if value:
            print('          .{0} {{{1}}},'.format(value_name, value), file=out)
        print('          .type {{{0}::{1}}}\n        }}'.format(type_name, type), end='', file=out)
        if i == len(list)-1:
            print('', file=out)
        else:
            print(',', file=out)
    print('      }', end='', file=out)

def parse_all_effects(out, card):
    try:
        print('    {{\n      .name {{\"{}\"}}'.format(card['name']), end='', file=out)
        if 'image' in card:
            print(',\n      .image {{\"{}\"}}'.format(card['image']), end='', file=out)
        if 'effects' in card:
            parse_effects(out, card['effects'], 'effects')
        if 'responses' in card:
            parse_effects(out, card['responses'], 'responses')
        if 'optional' in card:
            parse_effects(out, card['optional'], 'optionals')
        if 'equip' in card:
            parse_effect_simple(out, card['equip'], 'equips', 'effect_value', 'equip_type')
        if 'tags' in card:
            parse_effect_simple(out, card['tags'], 'tags', 'tag_value', 'tag_type')
        if 'expansion' in card:
            print(',\n      .expansion {{{}}}'.format(enum_flags(card['expansion'], 'card_expansion_type')), end='', file=out)
        if 'deck' in card:
            print(',\n      .deck {{card_deck_type::{}}}'.format(card['deck']), end='', file=out)
        if 'modifier' in card:
            print(',\n      .modifier {{card_modifier_type::{}}}'.format(card['modifier']), end='', file=out)
        if 'mth_effect' in card:
            print(',\n      .mth_effect {{mth_type::{}}}'.format(card['mth_effect']), end='', file=out)
        if 'mth_response' in card:
            print(',\n      .mth_response {{mth_type::{}}}'.format(card['mth_response']), end='', file=out)
        if 'equip_target' in card:
            print(',\n      .equip_target {{target_player_filter::{}}}'.format(card['equip_target']), end='', file=out)
        if 'color' in card:
            print(',\n      .color {{card_color_type::{}}}'.format(card['color']), end='', file=out)
    except RuntimeError as e:
        raise RuntimeError(f"Error in card {card['name']}:\n{e}")

def parse_file(out, data):
    hidden_cards = []

    print(
        "// AUTO GENERATED FILE\n\n"
        "#include \"game/card_data.h\"\n\n"
        "namespace banggame {\n\n"
        "using namespace enums::flag_operators;\n\n"
        "const all_cards_t all_cards {",
        file=out
    )

    print('  .deck {', file=out)
    for card in data['main_deck']:
        card['deck'] = 'main_deck'
        for sign in card['signs']:
            parse_all_effects(out, card)
            print(',\n      .sign {{{}}}\n    }},'.format(parse_sign(sign)), file=out)
    print('  },', file=out)

    print('  .characters {', file=out)
    for card in data['character']:
        card['deck'] = 'character'
        parse_all_effects(out, card)
        print('    },', file=out)
    print('  },', file=out)

    print('  .goldrush {', file=out)
    for card in data['goldrush']:
        set_expansion(card, 'goldrush')
        card['deck'] = 'goldrush'
        count = card['count'] if 'count' in card else 1
        for _ in range(count):
            if is_hidden(card):
                hidden_cards.append(card)
            else:
                parse_all_effects(out, card)
                print('\n    },', file=out)
    print('  },', file=out)

    print('  .highnoon {', file=out)
    for card in data['highnoon']:
        set_expansion(card, 'highnoon')
        card['deck'] = 'highnoon'
        if is_hidden(card):
            hidden_cards.append(card)
        else:
            parse_all_effects(out, card)
            print('\n    },', file=out)
    print('  },', file=out)

    print('  .fistfulofcards {', file=out)
    for card in data['fistfulofcards']:
        set_expansion(card, 'fistfulofcards')
        card['deck'] = 'fistfulofcards'
        if is_hidden(card):
            hidden_cards.append(card)
        else:
            parse_all_effects(out, card)
            print('\n    },', file=out)
    print('  },', file=out)

    print('  .wildwestshow {', file=out)
    for card in data['wildwestshow']:
        set_expansion(card, 'wildwestshow')
        card['deck'] = 'wildwestshow'
        if is_hidden(card):
            hidden_cards.append(card)
        else:
            parse_all_effects(out, card)
            print('\n    },', file=out)
    print('  },', file=out)

    print('  .specials {', file=out)
    for card in data['specials']:
        if is_hidden(card):
            hidden_cards.append(card)
        else:
            parse_all_effects(out, card)
            print('\n    },', file=out)
    print('  },', file=out)

    print('  .hidden {', file=out)
    for card in hidden_cards:
        parse_all_effects(out, card)
        print('\n    },', file=out)
    print('  },', file=out)

    print('};\n\n}', file=out)

def main():
    if len(sys.argv) < 3:
        print(f'Usage: {sys.argv[0]} bang_cards.yml bang_cards.cpp')
        exit(1)
    
    with open(sys.argv[1], 'r') as ifile:
        with open(sys.argv[2], 'w') as ofile:
            parse_file(ofile, yaml.safe_load(ifile))

if __name__ == '__main__':
    main()