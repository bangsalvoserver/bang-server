import sys

class CppDeclaration:
    def __init__(self, object_name, object_value, namespace_name = None):
        self.object_name = object_name
        self.object_value = object_value
        self.namespace_name = namespace_name

class CppObject:
    def __init__(self, **value):
        self.value = value
    
    def __eq__(self, value):
        return isinstance(value, CppObject) \
            and self.value == value.value

class CppSpan:
    def __init__(self, type_name, value):
        self.type_name = type_name
        self.value = value

    def __eq__(self, value):
        return isinstance(value, CppSpan) \
            and self.type_name == value.type_name \
            and self.value == value.value

class CppStaticMap:
    def __init__(self, key_type, value_type, value):
        self.key_type = key_type
        self.value_type = value_type
        self.value = value
    
    def __eq__(self, value):
        return isinstance(value, CppStaticMap) \
            and self.key_type == value.key_type \
            and self.value_type == value.value_type \
            and self.value == value.value

class CppEnum:
    def __init__(self, enum_name, value):
        self.enum_name = enum_name
        self.value = value
    
    def __str__(self):
        return f'{self.enum_name}::{self.value}'
    
    def __eq__(self, value):
        return isinstance(value, CppEnum) \
            and self.enum_name == value.enum_name \
            and self.value == value.value

class CppLiteral:
    def __init__(self, value):
        self.value = value
    
    def __str__(self):
        return self.value
    
    def __eq__(self, value):
        return isinstance(value, CppLiteral) \
            and self.value == value.value

SPACE = '  '

def chunks(lst, n):
    for i in range(0, len(lst), n):
        yield lst[i:i + n]

def convert_declaration(declaration: CppDeclaration, indent = 0):
    extra_declarations = []
    extra_declaration_count = 0

    def get_extra_declaration_name(object_value, object_type):
        nonlocal extra_declarations, extra_declaration_count

        for name, type_name, value in extra_declarations:
            if value == object_value and type_name == object_type:
                return name

        span_name = f"value_{extra_declaration_count}"
        extra_declaration_count += 1
        
        extra_declarations.append((span_name, object_type, object_value))
        return span_name

    def traverse_declaration(object_value):
        if isinstance(object_value, CppObject):
            return CppObject(**{
                key: traverse_declaration(value) for key, value in object_value.value.items()
            })
        elif isinstance(object_value, CppSpan):
            if not object_value.value: return []
            return CppLiteral(get_extra_declaration_name(traverse_declaration(object_value.value), object_value.type_name))
        elif isinstance(object_value, CppStaticMap):
            return CppLiteral(get_extra_declaration_name(CppStaticMap(
                key_type=object_value.key_type,
                value_type=object_value.value_type,
                value=traverse_declaration(object_value.value)
            ), object_type=(object_value.key_type, object_value.value_type)))
        elif isinstance(object_value, list):
            return [traverse_declaration(value) for value in object_value]
        elif isinstance(object_value, tuple):
            return tuple(traverse_declaration(value) for value in object_value)
        elif isinstance(object_value, bytes):
            return CppLiteral(get_extra_declaration_name(object_value, 'uint8_t'))
        else:
            return object_value
    
    def gen_extra_declaration(name, type_name, value):
        if isinstance(value, CppStaticMap):
            return CppDeclaration(
                object_name=f"static const auto {name}",
                object_value=value
            )
        return CppDeclaration(
            object_name=f"static const {type_name} {name}[]",
            object_value=value
        )

    def object_to_string(object_value, indent = 0):
        if isinstance(object_value, CppDeclaration):
            return f"{SPACE * indent}{object_value.object_name} {object_to_string(object_value.object_value, indent)};\n"
        elif isinstance(object_value, CppObject):
            if not object_value.value: return '{}'
            return '{\n' + ',\n'.join(f"{SPACE * (indent + 1)}.{key} {object_to_string(value, indent + 1)}" for key, value in object_value.value.items() if value is not None) + '\n' + (SPACE * indent) + '}'
        elif isinstance(object_value, dict):
            if not object_value: return '{}'
            return '{\n' + ',\n'.join(f"{SPACE * (indent + 1)}{{{object_to_string(key)}, {object_to_string(value)}}}" for key, value in object_value.items()) + '\n' + (SPACE * indent) + '}'
        elif isinstance(object_value, list):
            if not object_value: return '{}'
            return '{\n' + ',\n'.join(f"{SPACE * (indent + 1)}{object_to_string(value, indent + 1)}" for value in object_value if value is not None) + '\n' + (SPACE * indent) + '}'
        elif isinstance(object_value, tuple):
            if not object_value: return '{}'
            return '{ ' + ', '.join(f"{object_to_string(value, indent)}" for value in object_value) + ' }'
        elif isinstance(object_value, CppStaticMap):
            if not object_value: return '{}'
            return f'{{utils::make_static_map<{object_value.key_type}, {object_value.value_type}>({{\n' + \
                ',\n'.join(f"{SPACE * (indent + 1)}{object_to_string(value, indent + 1)}" for value in object_value.value if value is not None) + \
                '\n' + (SPACE * indent) + '})}'
        elif isinstance(object_value, str):
            return f'{{\"{object_value}\"sv}}'
        elif isinstance(object_value, bool):
            return f"{{{'true' if object_value else 'false'}}}"
        elif isinstance(object_value, bytes):
            if not object_value: return '{}'
            return '{\n' + ',\n'.join(SPACE * (indent + 1) + ','.join(f'0x{byte:02x}' for byte in line) for line in chunks(object_value, 32)) + '\n' + (SPACE * indent) + '}'
        else:
            return f'{{{object_value}}}'
    
    transformed = CppDeclaration(
        object_name=declaration.object_name,
        object_value=traverse_declaration(declaration.object_value),
        namespace_name=declaration.namespace_name
    )

    return "\n".join(
        object_to_string(decl, indent)
        for decl in [*(gen_extra_declaration(*tup) for tup in extra_declarations), transformed]
    )

def as_list(value):
    if isinstance(value, list):
        for elem in value:
            yield elem
    else:
        yield value
        
def print_cpp_file(declaration: CppDeclaration, include_filenames = None, file=sys.stdout):
    print("// AUTO GENERATED FILE\n", file=file)
    for filename in as_list(include_filenames):
        if filename.startswith('<'):
            print(f"#include {filename}\n", file=file)
        else:
            print(f"#include \"{filename}\"\n", file=file)
    
    indent = 0
    if declaration.namespace_name:
        print(f"namespace {declaration.namespace_name} {{\n", file=file)
        indent += 1
    print((SPACE * indent) + "using namespace std::string_view_literals;\n", file=file)
    print(convert_declaration(declaration, indent), file=file)

    if declaration.namespace_name:
        print('}', file=file)
