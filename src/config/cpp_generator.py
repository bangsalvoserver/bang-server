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

class CppStatic:
    def __init__(self, type_name, value, pointer = False):
        self.type_name = type_name
        self.value = value
        self.pointer = pointer

    def __eq__(self, value):
        return isinstance(value, CppStatic) \
            and self.type_name == value.type_name \
            and self.value == value.value \
            and self.pointer == value.pointer

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
    
    def __hash__(self):
        return hash((self.enum_name, self.value))

class CppLiteral:
    def __init__(self, value):
        self.value = value
    
    def __str__(self):
        return self.value
    
    def __eq__(self, value):
        return isinstance(value, CppLiteral) \
            and self.value == value.value
    
    def __hash__(self):
        return hash(self.value)

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
        match object_value:
            case CppObject():
                return CppObject(**{
                    key: traverse_declaration(value) for key, value in object_value.value.items()
                })
            case CppStatic():
                if not object_value.value: return []
                decl_name = get_extra_declaration_name(traverse_declaration(object_value.value), object_value.type_name)
                if object_value.pointer:
                    decl_name = '&' + decl_name
                return CppLiteral(decl_name)
            case CppStaticMap():
                return CppLiteral(get_extra_declaration_name(CppStaticMap(
                    key_type=object_value.key_type,
                    value_type=object_value.value_type,
                    value=traverse_declaration(object_value.value)
                ), object_type=(object_value.key_type, object_value.value_type)))
            case list() | set():
                return [traverse_declaration(value) for value in object_value]
            case dict():
                return {key: traverse_declaration(value) for key, value in object_value.items()}
            case tuple():
                return tuple(traverse_declaration(value) for value in object_value)
            case bytes():
                return CppLiteral(get_extra_declaration_name(object_value, 'uint8_t'))
            case _:
                return object_value
    
    def gen_extra_declaration(name, type_name, value):
        match value:
            case CppStaticMap():
                return CppDeclaration(
                    object_name=f"static const auto {name}",
                    object_value=value
                )
            case list() | set() | bytes():
                return CppDeclaration(
                    object_name=f"static const {type_name} {name}[]",
                    object_value=value
                )
            case _:
                return CppDeclaration(
                    object_name=f"static const {type_name} {name}",
                    object_value=value
                )

    def object_to_string(object_value, indent = 0):
        match object_value:
            case CppDeclaration():
                return f"{SPACE * indent}{object_value.object_name} {object_to_string(object_value.object_value, indent)};\n"
            case CppObject():
                filtered_items = [(key, value) for key, value in (object_value.value or {}).items() if value is not None]
                if not filtered_items: return '{}'
                return '{\n' + ',\n'.join(f"{SPACE * (indent + 1)}.{key} {object_to_string(value, indent + 1)}" for key, value in filtered_items) + '\n' + (SPACE * indent) + '}'
            case dict():
                if not object_value: return '{}'
                return '{\n' + ',\n'.join(f"{SPACE * (indent + 1)}{{{object_to_string(key)}, {object_to_string(value)}}}" for key, value in object_value.items()) + '\n' + (SPACE * indent) + '}'
            case list() | set():
                filtered_items = [value for value in (object_value or []) if value is not None]
                if not filtered_items: return '{}'
                return '{\n' + ',\n'.join(f"{SPACE * (indent + 1)}{object_to_string(value, indent + 1)}" for value in filtered_items) + '\n' + (SPACE * indent) + '}'
            case tuple():
                if not object_value: return '{}'
                return '{ ' + ', '.join(f"{object_to_string(value, indent)}" for value in object_value) + ' }'
            case CppStaticMap():
                if not object_value: return '{}'
                return f'{{utils::make_static_map<{object_value.key_type}, {object_value.value_type}>({{\n' + \
                    ',\n'.join(f"{SPACE * (indent + 1)}{object_to_string(pair, indent + 1)}" for pair in object_value.value.items()) + \
                    '\n' + (SPACE * indent) + '})}'
            case str():
                return f'{{\"{object_value}\"sv}}'
            case bool():
                return f"{{{'true' if object_value else 'false'}}}"
            case bytes():
                if not object_value: return '{}'
                return '{\n' + ',\n'.join(SPACE * (indent + 1) + ','.join(f'0x{byte:02x}' for byte in line) for line in chunks(object_value, 32)) + '\n' + (SPACE * indent) + '}'
            case _:
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
        
def print_cpp_file(declaration: CppDeclaration, include_filenames = None, file=sys.stdout):
    print("// AUTO GENERATED FILE\n", file=file)
    for filename in (include_filenames if isinstance(include_filenames, list) else [include_filenames]):
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
