import sys

class CppDeclaration:
    def __init__(self, object_name, object_value, namespace_name = None):
        self.object_name = object_name
        self.object_value = object_value
        self.namespace_name = namespace_name

class CppObject:
    def __init__(self, **value):
        self.value = value

class CppEnum:
    def __init__(self, enum_name, value):
        self.enum_name = enum_name
        self.value = value
    
    def __str__(self):
        return f'{self.enum_name}::{self.value}'

class CppLiteral:
    def __init__(self, value):
        self.value = value
    
    def __str__(self):
        return self.value

SPACE = '  '

def chunks(lst, n):
    for i in range(0, len(lst), n):
        yield lst[i:i + n]

def as_list(value):
    if isinstance(value, list):
        for elem in value:
            yield elem
    else:
        yield value

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
    elif isinstance(object_value, bytes):
        if not object_value: return '{}'
        return '{\n' + ',\n'.join(SPACE * (indent + 1) + ','.join(f'0x{byte:02x}' for byte in line) for line in chunks(object_value, 32)) + '\n' + (SPACE * indent) + '}'
    else:
        return f'{{{object_value}}}'

def print_cpp_file(object_values, include_filenames = None, file=sys.stdout):
    print("// AUTO GENERATED FILE\n", file=file)
    for filename in as_list(include_filenames):
        if filename.startswith('<'):
            print(f"#include {filename}\n", file=file)
        else:
            print(f"#include \"{filename}\"\n", file=file)
    
    indent = 0
    for declaration in as_list(object_values):
        if declaration.namespace_name:
            print(f"namespace {declaration.namespace_name} {{\n", file=file)
            indent += 1
        print(f"{SPACE * indent}{declaration.object_name} {object_to_string(declaration.object_value, indent)};\n", file=file)

        if declaration.namespace_name:
            print('}', file=file)
            indent -= 1
