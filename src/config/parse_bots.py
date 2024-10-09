import re
import sys
import yaml_custom as yaml
from PIL import Image
from cpp_generator import print_cpp_file, CppEnum, CppObject, CppLiteral

INCLUDE_FILENAMES = ['net/bot_info.h']
OBJECT_DECLARATION = 'bot_info_t banggame::bot_info'

def chunks(lst, n):
    for i in range(0, len(lst), n):
        yield lst[i:i + n]

def sdl_image_pixels(filename, propic_size):
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

        pixels = image.resize((w, h)).convert('RGBA').tobytes('raw', 'RGBA', 0, 1)
        pixels_str = ',\n'.join(','.join(f'0x{byte:02x}' for byte in line) for line in chunks(pixels, 32))

        decl_name = re.sub(r'[^\w]', '_', filename)

        return {
            'name': decl_name,
            'declaration': f'static const uint8_t {decl_name}[] = {{\n{pixels_str}}};',
            'width': w,
            'height': h
        }
    
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
        return CppLiteral(f'BUILD_BOT_RULE({rule_name}, {rule_args})')
    else:
        return CppLiteral(f'BUILD_BOT_RULE({rule_name})')

def parse_settings(settings):
    return CppObject(
        allow_timer_no_action = settings['allow_timer_no_action'],
        max_random_tries = settings['max_random_tries'],
        bypass_prompt_after = settings['bypass_prompt_after'],
        response_rules = [parse_bot_rule(value) for value in settings['response_rules']],
        in_play_rules = [parse_bot_rule(value) for value in settings['in_play_rules']]
    )

def parse_file(data):
    propic_size = data['propic_size']
    declarations = []
    propics = []
    for filename in data['propics']:
        propic = sdl_image_pixels(filename, propic_size)
        declarations.append(propic['declaration'])
        propics.append(CppObject(
            width = propic['width'],
            height = propic['height'],
            pixels = CppLiteral(propic['name'])
        ))
    
    return CppObject(
        propic_size = propic_size,
        names = data['names'],
        propics = propics,
        settings = parse_settings(data['settings'])
    ), declarations

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print(f'Usage: {sys.argv[0]} bot_info.yml bot_info.cpp')
        sys.exit(1)

    with open(sys.argv[1], 'r', encoding='utf8') as file:
        bot_info, declarations = parse_file(yaml.safe_load(file)['bots'])
    
    if sys.argv[2] == '-':
        print_cpp_file(bot_info, OBJECT_DECLARATION,
            include_filenames=INCLUDE_FILENAMES,
            declarations=declarations,
            file=sys.stdout)
    else:
        with open(sys.argv[2], 'w', encoding='utf8') as file:
            print_cpp_file(bot_info, OBJECT_DECLARATION,
                include_filenames=INCLUDE_FILENAMES,
                declarations=declarations,
                file=file)