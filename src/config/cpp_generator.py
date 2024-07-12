import sys
import re

class CppObject:
    def __init__(self, **value):
        self.value = value

class CppEnum:
    def __init__(self, enum_name, values):
        self.enum_name = enum_name
        self.values = values.split()
    
    def __str__(self):
        return ', '.join(f'{self.enum_name}::{value}' for value in self.values)

class CppLiteral:
    def __init__(self, value):
        self.value = value
    
    def __str__(self):
        return self.value

SPACE = '  '

def object_to_string(object_value, indent = 0):
    if isinstance(object_value, CppObject):
        if not object_value: return '{}'
        return '{\n' + ',\n'.join(f"{SPACE * (indent + 1)}.{key} {object_to_string(value, indent + 1)}" for key, value in object_value.value.items() if value is not None) + '\n' + (SPACE * indent) + '}'
    elif isinstance(object_value, dict):
        if not object_value: return '{}'
        return '{\n' + ',\n'.join(f"{SPACE * (indent + 1)}{{{object_to_string(key)}, {object_to_string(value)}}}" for key, value in object_value.items()) + '\n' + (SPACE * indent) + '}'
    elif isinstance(object_value, list):
        if not object_value: return '{}'
        return '{\n' + ',\n'.join(f"{SPACE * (indent + 1)}{object_to_string(value, indent + 1)}" for value in object_value if value is not None) + '\n' + (SPACE * indent) + '}'
    elif isinstance(object_value, str):
        return f'{{\"{object_value}\"}}'
    elif isinstance(object_value, bool):
        return f"{{{'true' if object_value else 'false'}}}"
    else:
        return f'{{{object_value}}}'

def print_cpp_file(object_value, object_declaration, include_filenames = None, declarations=None, file=sys.stdout):
    match = re.match(r'^(\w+) (?:(\w+)::)?(\w+)$', object_declaration)

    object_type = match.group(1)
    namespace_name = match.group(2)
    object_name = match.group(3)

    print("// AUTO GENERATED FILE\n", file=file)
    if isinstance(include_filenames, list):
        for filename in include_filenames:
            print(f"#include \"{filename}\"\n", file=file)
    elif isinstance(include_filenames, str):
        print(f"#include \"{include_filenames}\"\n", file=file)
    indent = 0
    if namespace_name:
        print(f"namespace {namespace_name} {{\n", file=file)
        indent += 1
    if declarations:
        print(SPACE * indent + declarations + '\n', file=file)
    print(f"{SPACE * indent}const {object_type} {object_name} {object_to_string(object_value, indent)};", file=file)

    if namespace_name:
        print('}', file=file)
