import sys
import yaml_custom as yaml
import base64
from PIL import Image
from cpp_generator import print_cpp_file

PROPIC_SIZE = 50
INCLUDE_FILENAMES = ['net/bot_info.h']
OBJECT_DECLARATION = 'bot_info_t banggame::bot_info'

def sdl_image_pixels(filename):
    with Image.open(filename) as image:
        w = image.width
        h = image.height

        if w > h:
            h = PROPIC_SIZE * h // w
            w = PROPIC_SIZE
        else:
            w = PROPIC_SIZE * w // h
            h = PROPIC_SIZE
        
        return {
            'width': w, 'height': h,
            'pixels': base64.b64encode(image.resize((w, h)).convert('RGBA').tobytes('raw', 'RGBA', 0, 1)).decode('utf8')
        }

def parse_file(data):
    return {
        'names': data['names'],
        'propics': [sdl_image_pixels(filename) for filename in data['propics']]
    }

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print(f'Usage: {sys.argv[0]} bot_info.yml bot_info.cpp')
        sys.exit(1)

    with open(sys.argv[1], 'r', encoding='utf8') as file:
        bot_info = parse_file(yaml.safe_load(file)['bots'])
    
    if sys.argv[2] == '-':
        print_cpp_file(bot_info, OBJECT_DECLARATION,
            include_filenames=INCLUDE_FILENAMES,
            file=sys.stdout)
    else:
        with open(sys.argv[2], 'w', encoding='utf8') as file:
            print_cpp_file(bot_info, OBJECT_DECLARATION,
                include_filenames=INCLUDE_FILENAMES,
                file=file)