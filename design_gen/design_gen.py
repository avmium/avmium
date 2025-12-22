import os, sys, getopt
import shutil
import yaml
from collections import defaultdict
from rich.console import Console
from rich.tree import Tree
from jinja2 import Template 
from jinja2 import Environment, FileSystemLoader

def print_structure(data, indent=0):
    space = " " * indent
    if isinstance(data, dict):
        for k, v in data.items():
            if isinstance(v, (dict, list)):
                print(f"{space}- {k}:")
                print_structure(v, indent + 4)
            else:
                print(f"{space}- {k}: {v}")
    elif isinstance(data, list):
        for i, item in enumerate(data):
            if isinstance(item, (dict, list)):
                print(f"{space}- [{i}]")
                print_structure(item, indent + 4)
            else:
                print(f"{space}- [{i}] {item}")
    else:
        print(f"{space}{data}")

class PreserveDuplicatesLoader(yaml.SafeLoader):
    """Loader that groups duplicate mapping keys into lists, but leaves singletons as scalars."""
    pass

def construct_mapping(loader, node, deep=False):
    # First, collect all values per key
    bucket = defaultdict(list)
    for key_node, value_node in node.value:
        key = loader.construct_object(key_node, deep=deep)
        value = loader.construct_object(value_node, deep=deep)
        bucket[key].append(value)

    # Then, collapse keys that only occurred once
    out = {}
    for k, vs in bucket.items():
        out[k] = vs[0] if len(vs) == 1 else vs
    return out

# Use our constructor for all mappings
PreserveDuplicatesLoader.add_constructor(
    yaml.resolver.BaseResolver.DEFAULT_MAPPING_TAG,
    construct_mapping
)

def add_func_to_c(file, af, template, func, mod_name):
    global cfile_template
    with open(file, "r", encoding="utf-8") as f:
        if any(line.strip() == "void " + mod_name +"_"+ func + "(void *opaque)" for line in f) == False:
            cfile_rendered = cfile_template.render(f=func, name=mod_name)
            af.write(cfile_rendered)

env = Environment(
    loader=FileSystemLoader('/home/muhmuz01/hostdir/amba_bridges/design_gen/templates'),
    extensions=['jinja2.ext.do']
)

config_file = ""
top_mod = "top"
prj_dir = ".."
path = prj_dir + '/generated'
argv = sys.argv[1:]
opts, args = getopt.getopt(argv, "hvf:", ["help", "file=","verbose", "fresh"])
fresh_structure = 0
# short options: -f <arg>, -v
# long options: --file <arg>, --verbose
default_signal_widths = {
    "apb" : {
        "addr"      : 32,
        "data"      : 32,
        "user_req"  : 128,
        "user_data" : 16,
        "user_resp" : 16
    },
    "axis" : {
        "tdata"     : 32,
        "tid"       : 32,
        "tdest"     : 32,
        "tuser"     : 32
    }
}

for opt, arg in opts:
    if opt in ("-h", "--help"):
        print("Usage: script.py -f <file> [-v]")
        sys.exit()
    elif opt in ("-f", "--file"):
        config_file = arg
    elif opt in ("-v", "--verbose"):
        print("Verbose mode on")
    elif opt in ("--fresh"):
        fresh_structure = 1

if config_file == '':
    print("Err: config file missing")
    sys.exit(1)

# Render the template and pass the variables
sc_template = env.get_template("sc_module.jinja2")
cmake_template = env.get_template("cmake.jinja2")
cfile_template = env.get_template("cfile.jinja2")
config_template = env.get_template("config.jinja2")
sv_template = env.get_template("sv.jinja2")
prj_top_template = env.get_template("prj_top.jinja2")
sc_top_template = env.get_template("sc_top.jinja2")
port_actions_template = env.get_template("port_actions.jinja2")

with open(config_file, 'r') as f:
    data = list(yaml.load_all(f, Loader=PreserveDuplicatesLoader))

config = {
    "prj_dir": prj_dir,
    "top_dir": path,
    "top_mod": top_mod,
    "sim_clk_cycles": 0,
    "wavedump": 0
}

if "config" in data[0]:
    node = data[0]["config"]
    if "prj_dir" in node:
        config["prj_dir"] = node["prj_dir"]
    if "top_dir" in node:
        config["top_dir"] = node["top_dir"]
        if fresh_structure:
            shutil.rmtree(config["top_dir"])

    if "top" in node:
        config["top_mod"] = node["top"]
    if "sim_clk_cycles" in node:
        config["sim_clk_cycles"] = node["sim_clk_cycles"]
    if "wavedump" in node:
        config["wavedump"] = node["wavedump"]
    config_rendered = config_template.render(conf=config)
    path = config["top_dir"]
    os.makedirs(path, exist_ok=True)
    with open(config["top_dir"] + "/Config.cmake", "w", encoding="utf-8") as f:
        f.write(config_rendered)
        f.close()
else:
   shutil.copy(prj_dir + "/config/Config.cmake", path + "/Config.cmake") 

port_name_dir = {}
for mod in data:
    if "def_mod" in mod and "port" in mod["def_mod"]:
        imod = mod["def_mod"]
        port_name_dir[imod["name"]] = []
        if isinstance(imod["port"], dict):
            imod["port"] = [imod["port"]]
        port_name_dir[imod["name"]] = {}
        for port in imod["port"]:
            port_name_dir[imod["name"]][port["name"]] = port["dir"]

for mod in data:
    cexists = True
    mod_tree = path
    if "def_mod" in mod:
        node = mod["def_mod"]
        if "memmap" in node:
            if "mem" in node["memmap"] and isinstance(node["memmap"]["mem"], dict):
                node["memmap"]["mem"] = [node["memmap"]["mem"]]
            if "port" in node["memmap"] and isinstance(node["memmap"]["port"], dict):
                node["memmap"]["port"] = [node["memmap"]["port"]]
        if "func" not in node:
            node["func"] = {}
        node["func"]["init"] = "callback"
        node["func"]["exit"] = "callback"
        if node["name"] != config["top_mod"]:
            mod_tree = mod_tree + "/" + node["name"]
            os.makedirs(mod_tree, exist_ok=True)
        if "port" in node:
            if isinstance(node["port"], dict):
                node["port"] = [node["port"]]
            for i, port in enumerate(node["port"]):
                node["func"][port["name"] + "_init"] = "callback"
                node["func"][port["name"] + "_exit"] = "callback"
                node["func"][port["name"] + "_resp"] = "callback"
                node["func"][port["name"] + "_user"] = "callback"
                if "func" in port:
                    for key, value in port["func"].items():
                        node["func"][port["name"] + "_" + key] = "callback"
                if "action" in port:
                    if isinstance(port["action"], dict):
                        node["port"][i]["action"] = [node["port"][i]["action"]]
                    for action in node["port"][i]["action"]:
                        node["func"][port["name"] + "_" + action["type"] + "_" + "{:#x}".format(action["start"]) + "_" + "{:#x}".format(action["end"])] = "callback"
        if "if" in node and isinstance(node["if"], dict):
            node["if"] = [node["if"]]
        if "mod" in node and isinstance(node["mod"], dict):
            node["mod"] = [node["mod"]]
        sc_rendered = sc_template.render(mod=node, dsw=default_signal_widths)
        cmake_rendered = cmake_template.render(mod=node, conf=config)
        with open(mod_tree + "/" + node["name"] + ".h", "w", encoding="utf-8") as f:
            f.write(sc_rendered)
            f.close()
        with open(mod_tree + "/CMakeLists.txt", "w", encoding="utf-8") as f:
            f.write(cmake_rendered)
            f.close()
        sv_rendered = sv_template.render(mod=node, port_conn=port_name_dir, dsw=default_signal_widths)
        with open(mod_tree + "/" + node["name"] + ".sv", "w", encoding="utf-8") as f:
            f.write(sv_rendered)
            f.close()
        prj_top_rendered = prj_top_template.render(mod=node, port_conn=port_name_dir, dsw=default_signal_widths)
        with open(mod_tree + "/prj_top.v", "w", encoding="utf-8") as f:
            f.write(prj_top_rendered)
            f.close()
        sc_top_rendered = sc_top_template.render(mod=node, dsw=default_signal_widths)
        with open(mod_tree + "/" + node["name"] + "_top.h", "w", encoding="utf-8") as f:
            f.write(sc_top_rendered)
            f.close()
        port_actions_rendered = port_actions_template.render(mod=node)
        with open(mod_tree + "/" + node["name"] + "_port_actions.c", "w", encoding="utf-8") as f:
            f.write(port_actions_rendered)
            f.close()
        if not os.path.exists(mod_tree + "/Config.cmake"):
            shutil.copy(config["top_dir"] + "/Config.cmake", mod_tree + "/Config.cmake") 
        if not os.path.exists(mod_tree + "/" + node["name"] + ".c"):
            cexists = False
        with open(mod_tree + "/" + node["name"] + ".c", "a", encoding="utf-8") as f:
            if not cexists:
                f.write('/* standard libs include */\n\n/* standard libs include (END) */\n#include "common.h"\n')
            for key, value in node["func"].items():
                add_func_to_c(mod_tree + "/" + node["name"] + ".c", f, cfile_template, key, node["name"])
            f.close()
