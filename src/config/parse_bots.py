import re
import sys
import yaml_custom as yaml
from PIL import Image
from cpp_generator import *

INCLUDE_FILENAMES = ['net/bot_info.h']

class ImagePixels:
    def __init__(self, filename, propic_size):
        with Image.open(filename) as image:
            w = image.width
            h = image.height

            if w > h:
                if propic_size < w:
                    h = propic_size * h // w
                    w = propic_size
            else:
                if propic_size < h:
                    w = propic_size * w // h
                    h = propic_size

            self.name = re.sub(r'[^\w]', '_', filename)
            self.pixels = image.resize((w, h)).convert('RGBA').tobytes('raw', 'RGBA', 0, 1)
            self.width = w
            self.height = h
    
def parse_bot_rule(value):
    match = re.match(
        r'^\s*(\w+)' # rule_name
        r'(?:\s*\((.+)\))?\s*$', # rule_args
        value
    )
    if not match:
        raise RuntimeError(f'Invalid rule string: {value}')
    
    rule_name = match.group(1)
    rule_args = match.group(2)
    if rule_args:
        return CppStatic('auto', CppLiteral(f'BUILD_BOT_RULE({rule_name}, {rule_args})'))
    else:
        return CppStatic('auto', CppLiteral(f'BUILD_BOT_RULE({rule_name})'))

def parse_settings(settings):
    return CppObject(
        max_random_tries = settings['max_random_tries'],
        bypass_empty_index = settings['bypass_empty_index'],
        bypass_unconditional_index = settings['bypass_unconditional_index'],
        repeat_card_nodes = settings['repeat_card_nodes'],
        response_rules = CppStatic('bot_rule', [parse_bot_rule(value) for value in settings['response_rules']]),
        in_play_rules = CppStatic('bot_rule', [parse_bot_rule(value) for value in settings['in_play_rules']])
    )

def parse_file(data):
    propic_size = data['propic_size']

    def gen_propic(filename):
        propic = ImagePixels(filename, propic_size)
        return (CppObject(
            width = propic.width,
            height = propic.height,
            pixels = propic.pixels
        ),)

    return CppDeclaration(
        object_name='const bot_info_t bot_info',
        object_value=CppObject(
            propic_size = propic_size,
            names = CppStatic('std::string_view', data['names']),
            propics = CppStatic('image_registry::registered_image', [
                gen_propic(filename) for filename in data['propics']
            ]),
            settings = parse_settings(data['settings'])
        ),
        namespace_name='banggame'
    )

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print(f'Usage: {sys.argv[0]} bot_info.yml bot_info.cpp')
        sys.exit(1)

    with open(sys.argv[1], 'r', encoding='utf8') as file:
        bot_info = parse_file(yaml.safe_load(file)['bots'])
    
    if sys.argv[2] == '-':
        print_cpp_file(bot_info, include_filenames=INCLUDE_FILENAMES, file=sys.stdout)
    else:
        with open(sys.argv[2], 'w', encoding='utf8') as file:
            print_cpp_file(bot_info, include_filenames=INCLUDE_FILENAMES, file=file)