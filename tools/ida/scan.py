from cp77ida import Item, Group, Output, scan
from pathlib import Path


# Defines patterns and output files
def patterns():
    return [
        Output(filename="src/App/Addresses.hpp", namespace="Reverse::Addresses", groups=[
            Group(name="AppearanceChanger", functions=[
                Item(name="ComputePlayerGarment",
                     pattern="48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 4C 89 64 24 20 55 41 56 41 57 48 8D 6C 24 C0"),
            ]),
            Group(name="AppearanceNameVisualTagsPreset", functions=[
                Item(name="GetVisualTags",
                     pattern="48 89 74 24 18 48 89 54 24 10 57 48 83 EC 40 49 8B F8 48 8D 54 24 20 4C 8D 44 24 58 48 83 C1 40 49 8B F1"),
            ]),
            Group(name="AppearanceResource", functions=[
                Item(name="FindAppearanceDefinition",
                     pattern="48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 41 56 41 57 48 83 EC 20 48 8B F1 45 8B F9 48 81 C1 F0 00 00 00 49 8B D8 4C 8B F2"),
            ]),
            Group(name="Engine", functions=[
                Item(name="Main",
                     pattern="40 53 48 81 EC ? ? ? ? FF 15 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ?"),
            ]),
            Group(name="Entity", functions=[
                Item(name="GetComponents",
                     pattern="48 83 C1 70 E9",
                     expected=9,
                     index=1),
                Item(name="ReassembleAppearance",
                     pattern="40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 88 48 81 EC 78 01 00 00 4C 8B EA 49 8B D9"),
            ]),
            Group(name="EntityTemplate", functions=[
                Item(name="FindAppearance",
                     pattern="40 53 48 83 EC 20 48 8B  D9 48 85 D2 74 ? 48 3B 15 ? ? ? ? 75 ? 48  8B 51 60 48 3B 15 ? ? ? ? 48 89 7C 24 30 75"),
            ]),
            Group(name="FactoryIndex", functions=[
                Item(name="LoadFactoryAsync",
                     pattern="48 89 5C 24 18 55 56 57 41 56 41 57 48 8D 6C 24 C9 48 81 EC A0 00 00 00 45 33 FF 48 89 55 DF 4D 8B F0 44 89 7D FB 48 8B D9 4C 89 7D E7"),
            ]),
            Group(name="GarmentAssembler", functions=[
                Item(name="AddItem",
                     pattern="48 89 5C 24 08 57 48 83 EC 60 49 8B F8 4C 8B C2 48 8D 54 24 40 E8 ? ? ? ? 48 8B 07",
                     expected=2,
                     index=0),
                Item(name="OverrideItem",
                     pattern="48 89 5C 24 08 57 48 83 EC 60 49 8B F8 4C 8B C2 48 8D 54 24 40 E8 ? ? ? ? 48 8B 07",
                     expected=2,
                     index=1),
                Item(name="RemoveItem",
                     pattern="48 89 5C 24 08 57 48 83 EC 40 49 8B F8 4C 8B C2 48 8D 54 24 20 E8 ? ? ? ? 44 8B 47"),
                Item(name="OnGameDetach",
                     pattern="40 53 48 83 EC 20 8B 51 14 48 8B D9 48 8B 49 08 E8 ? ? ? ? C7 43 14 00 00 00 00",
                     expected=2,
                     index=1),
            ]),
            Group(name="InkSpawner", functions=[
                Item(name="SpawnFromExternal",
                     pattern="48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 55 41 56 41 57 48 8D 6C 24 C1 48 81 EC F0 00 00 00"),
            ]),
            Group(name="Localization", functions=[
                Item(name="LoadOnScreens",
                     pattern="40 55 53 57 48 8B EC 48 81 EC 80 00 00 00 0F 57 C0 48 8B D9 F3 0F 7F 45 A0 48 85 D2 75 17 33 FF 48 8B C1 48 89 39 48 89 79 08 48 81 C4 80 00 00",
                     expected=2),
            ]),
            Group(name="TagList", functions=[
                Item(name="MergeWith",
                     pattern="48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 48 8B 1A 48 8B F1 8B 42 0C 48 8D 3C C3 48 3B DF 74"),
            ]),
        ]),
    ]


# Defines base output dir
def output_dir():
    cwd = Path(__file__).resolve().parent
    return cwd.parent.parent  # 2 levels up


scan(patterns(), output_dir(), __file__)
