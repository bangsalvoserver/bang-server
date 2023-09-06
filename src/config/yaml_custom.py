import yaml
from pathlib import Path

def my_compose_document(self):
    self.get_event()
    node = self.compose_node(None, None)
    self.get_event()
    # self.anchors = {}    # <<<< commented out
    return node

yaml.SafeLoader.compose_document = my_compose_document

def yaml_include(loader, node):
    path = Path(loader.name).parent / node.value
    with open(path) as inputfile:
        return my_safe_load(inputfile, master=loader)

yaml.add_constructor("!include", yaml_include, Loader=yaml.SafeLoader)

def my_safe_load(stream, Loader=yaml.SafeLoader, master=None):
    loader = Loader(stream)
    if master is not None:
        loader.anchors = master.anchors
    try:
        return loader.get_single_data()
    finally:
        loader.dispose()

def safe_load(stream):
    return my_safe_load(stream)