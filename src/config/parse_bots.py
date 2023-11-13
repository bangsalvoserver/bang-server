import sys
import yaml_custom as yaml
import base64
from PIL import Image
from cpp_generator import print_cpp_file

INCLUDE_FILENAMES = ['net/bot_info.h']
OBJECT_DECLARATION = 'bot_info_t banggame::bot_info'

def sdl_image_pixels(propic):
    with Image.open(propic) as image:
        return {
            'width': image.width,
            'height': image.height,
            'pixels': base64.b64encode(image.tobytes()).decode('utf8')
        }

def parse_file(data):
    return {
        'names': data['names'],
        'propics': [sdl_image_pixels(propic) for propic in data['propics']]
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