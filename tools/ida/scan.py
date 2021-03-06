from cp77ida import Item, Group, Output, scan
from pathlib import Path


# Defines patterns and output files
def patterns():
    return [
        Output(filename="lib/Engine/Addresses.hpp", namespace="Engine::Addresses", groups=[
            Group(name="Engine", functions=[
                Item(name="Main",
                     pattern="40 53 48 81 EC ? ? ? ? FF 15 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ?"),
            ]),
            Group(name="ResourceReference", functions=[
                Item(name="Load",
                     pattern="48 89 5C 24 10 57 48 83 EC 30 48 8D 59 08 48 8B F9 48 8B 0B 48 85 C9"),
                Item(name="Fetch",
                     pattern="40 53 48 83 EC 20 48 8B D9 E8 ? ? ? ? 48 8B 4B 08 48 85 C9 74 0A"),
                Item(name="Reset",
                     pattern="48 83 EC 38 45 33 C0 4C 89 01 48 8B 51 10 48 8B 41 08 48 89 54 24 28 4C 89 41 10 48 89 44 24 20"),
            ]),
        ]),
        Output(filename="src/Reverse/Addresses.hpp", namespace="Reverse::Addresses", groups=[
            Group(functions=[
                Item(name="FactoryIndex_LoadFactoryAsync",
                     pattern="48 89 5C 24 18 55 56 57 41 56 41 57 48 8D 6C 24 C9 48 81 EC A0 00 00 00 45 33 FF 48 89 55 DF 4D 8B F0 44 89 7D FB 48 8B D9 4C 89 7D E7"),
                Item(name="Localization_LoadOnScreens",
                     pattern="40 55 53 57 48 8B EC 48 81 EC 80 00 00 00 0F 57 C0 48 8B D9 F3 0F 7F 45 A0 48 85 D2 75 17 33 FF 48 8B C1 48 89 39 48 89 79 08 48 81 C4 80 00 00",
                     expected=2),
            ]),
        ]),
    ]


# Defines base output dir
def output_dir():
    cwd = Path(__file__).resolve().parent
    return cwd.parent.parent  # 2 levels up


scan(patterns(), output_dir())
