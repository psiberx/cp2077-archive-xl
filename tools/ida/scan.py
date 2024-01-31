from cp77ida import Item, Group, Output, scan
from pathlib import Path


# Defines patterns and output files
def patterns():
    return [
        Output(filename="src/Red/Addresses.hpp", namespace="Red::Addresses", groups=[
            Group(functions=[
                Item(name="Main",
                     pattern="40 55 53 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? FF 15 ? ? ? ? E8"),
            ]),
            Group(name="AnimatedComponent", functions=[
                Item(name="InitializeAnimations",
                     pattern="48 89 5C 24 10 48 89 74 24 18 48 89  7C 24 20 55 48 8B EC 48 83 EC 50 48 8B D9 48 8D 05 ? ? ? ? 33 C9"),
            ]),
            Group(name="AppearanceChanger", functions=[
                Item(name="ComputePlayerGarment",
                     pattern="48 8B C4 48 89 58 ? 48 89 70 ? 48 89 78 ? 4C 89 70 ? 55 48 8D 68 ? 48 81 EC ? ? ? ? 4C 8B F1 49 8B D9 48 8D 4D ? 49 8B F8  48 8B F2 E8"),
                Item(name="GetBaseMeshOffset",
                     pattern="48 8B C4 48 89 58 ? 48 89 70 ? 48 89 78 ? 55 41 56 41 57 48 8D 68 ? 48 81 EC ? ? ? ? 48 8D 05 ? ? ? ? 4C 8B FA 48 8B 11 48 89 45 ?"),
                Item(name="GetSuffixValue",
                     pattern="48 8B C4 48 89 58 ? 48 89 70 ? 4C 89 48 ? 55 57 41 54 41 56 41 57 48 8D 68 ? 48 81 EC ? ? ? ? 33 F6 4D 8B F0 4C 8B F9 48 85 D2"),
                Item(name="RegisterPart",
                     pattern="48 8B C4 48 89 58 ? 48 89 70 ? 48 89 78 ? 4C 89 70 ? 41 57 48 83 EC ? 48 8D 99 ? ? ? ? 48 8B F9 48 8B CB 4D 8B F1 4D 8B F8 48 8B F2 E8"),
                Item(name="SelectAppearanceName",
                     pattern="48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 41 56 41 57 48 8B EC 48 83 EC ? 48 83 21 00 4D 8B F8 4C 8B 45 ? 49 8B F1 48 8B DA"),
            ]),
            Group(name="AppearanceChangeSystem", functions=[
                Item(name="ChangeAppearance",
                     pattern="48 8B C4 48 89 58 ? 48 89 70 ? 48 89 78 ? 55 41 54 41 55 41 56 41 57 48 8D 68 ? 48 81 EC ? ? ? ? 48 8B 42 ? 4C 8B EA 4C 8B F1"),
            ]),
            Group(name="AppearanceNameVisualTagsPreset", functions=[
                Item(name="GetVisualTags",
                     pattern="48 8B C4 48 89 58 ? 48 89 50 ? 57 48 83 EC ? 49 8B D8 48 8D 50 ? 4C 8D 40 ? 48 83 C1 ? 49 8B F9 E8 ? ? ? ? 48 8B 4C 24"),
            ]),
            Group(name="AppearanceResource", functions=[
                Item(name="FindAppearanceDefinition",
                     pattern="48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 48 8D B9 ? ? ? ? 48 8B F1 48 8B CF 45 8B F9 49 8B D8 4C 8B F2 E8"),
            ]),
            Group(name="AttachmentSlots", functions=[
                Item(name="InitializeSlots",
                     pattern="48 8B C4 48 89 58 ? 48 89 70 ? 48 89 78 ? 55 48 8D 68 ? 48 81 EC ? ? ? ? 83 7A ? ? 48 8B DA 0F 84"),
                Item(name="IsSlotEmpty",
                     pattern="48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D B9 ? ? ? ? 48 8B F1 48 8B CF 48 8B DA E8 ? ? ? ? 48 8B D3 48 8B CE E8 ? ? ? ? 33 ? 48 85 C0 74",
                     expected=3,
                     index=0),
                Item(name="IsSlotSpawning",
                     pattern="48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D B9 ? ? ? ? 48 8B F1 48 8B CF 48 8B DA E8 ? ? ? ? 48 8B D3 48 8B CE E8 ? ? ? ? 33 ? 48 85 C0 74",
                     expected=3,
                     index=1),
            ]),
            Group(name="CBaseEngine", functions=[
                Item(name="LoadGatheredResources",
                     pattern="48 83 EC ? 48 8B 01 FF 90 ? ? ? ? 33 C9 84 C0 0F 95 C1 E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? B0 01 48 83 C4 ? C3"),
            ]),
            Group(name="CharacterCustomizationFeetController", functions=[
                Item(name="CheckState",
                     pattern="48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 48 8B 41 ? 48 8B D9 48 8B F2 4D 8B F0 48 8B 15"),
            ]),
            Group(name="CharacterCustomizationGenitalsController", functions=[
                Item(name="OnAttach",
                     pattern="F6 05 ? ? ? ? 01 0F 85 ? ? ? ? E9"),
                Item(name="CheckState",
                     pattern="4C 89 44 24 ? 48 89 54 24 ? 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 01 48 8B FA 45"),
            ]),
            Group(name="CharacterCustomizationHairstyleController", functions=[
                Item(name="OnDetach",
                     pattern="48 8B C4 48 89 58 ? 48 89 78 ? 4C 89 70 ? 55 48 8D 68 ? 48 81 EC ? ? ? ? 48 8B FA 48 8B D9 48 81 C1 ? ? ? ?"),
                Item(name="CheckState",
                     pattern="48 89 54 24 ? 48 89 4C 24 ? 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 41 ? 48 8B D9"),
            ]),
            Group(name="CharacterCustomizationHelper", functions=[
                Item(name="GetHairColor",
                     pattern="4C 8B DC 49 89 5B ? 49 89 6B ? 49 89 73 ? 57 48 83 EC ? 48 8B F1 48 8B 0A 49 8D 53 ? 48 8B 01 FF 90"),
            ]),
            Group(name="CharacterCustomizationSystem", functions=[
                Item(name="Initialize",
                     pattern="48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC ? 4C 8B E2 4D 8B F1 48 8D 55"),
                Item(name="Uninitialize",
                     pattern="40 53 48 83 EC ? 48 8B D9 48 8D 91 ? ? ? ? 48 8D 4C 24 ? E8 ? ? ? ? 48 8B CB E8 ? ? ? ? B2 01 48 8B CB E8"),
                Item(name="EnsureState",
                     pattern="48 8B C4 48 89 58 ? 48 89 70 ? 48 89 78 ? 55 41 56 41 57 48 8D 68 ? 48 81 EC ? ? ? ? 4C 8B F1 48 8B F2 48 8B 0A"),
                Item(name="InitializeAppOption",
                     pattern="48 89 5C 24 ? 48 89 74 24 ? 55 57 41 54 41 56 41 57 48 8B EC 48 83 EC ? 48 8B F1 44 8B E2 49 8B 08 48 8D 55"),
                Item(name="InitializeMorphOption",
                     pattern="48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 41 54 41 55 41 56 41 57 48 8B EC 48 81 EC ? ? ? ? 4C 8B E9 48 8B DA"),
                Item(name="InitializeSwitcherOption",
                     pattern="48 8B C4 48 89 58 ? 48 89 70 ? 48 89 78 ? 4C 89 40 ? 55 41 54 41 55 41 56 41 57 48 8D 68 ? 48 81 EC ? ? ? ? 48 8B F1"),
            ]),
            Group(name="CMesh", functions=[
                Item(name="GetAppearance",
                     pattern="40 53 48 83 EC ? 48 8B D9 E8 ? ? ? ? 48 83 38 00 74 ? 48 83 C4 ? 5B C3"),
            ]),
            Group(name="Entity", functions=[
                Item(name="ReassembleAppearance",
                     pattern="48 8B C4 48 89 58 ? 48 89 70 ? 48 89 78 ? 55 41 56 41 57 48 8D 68 ? 48 81 EC ? ? ? ? 4C 8B F2 49 8B F9 48"),
            ]),
            Group(name="EntityTemplate", functions=[
                Item(name="FindAppearance",
                     pattern="40 53 48 83 EC ? 48 8B D9 48 85 D2 74 ? 48 3B 15 ? ? ? ? 75 ? 48 8B 51 ? 48 3B 15 ? ? ? ? 74"),
            ]),
            Group(name="FactoryIndex", functions=[
                Item(name="LoadFactoryAsync",
                     pattern="48 89 5C 24 ? 48 89 7C 24 ? 55 48 8B EC 48 81 EC ? ? ? ? 48 83 65 ? 00 49 8B F8 83 65 ? 00 4C 8D 45 ? 48 83 65 ? 00"),
                Item(name="ResolveResource",
                     pattern="40 53 48 83 EC ? 48 8B DA 48 8D 54 24 ? E8 ? ? ? ? 48 8B 08 48 8B C3 48 89 0B 48 83 C4 ? 5B C3"),
            ]),
            Group(name="GameApplication", functions=[
                Item(name="InitResourceDepot",
                     pattern="48 89 5C 24 ? 48 89 74 24 ? 55 57 41 56 48 8D 6C 24 ? 48 81 EC ? ? ? ? 44 8B 82 ? ? ? ? 45 33 F6"),
            ]),
            Group(name="GarmentAssembler", functions=[
                Item(name="FindState",
                     pattern="48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 83 EC ? 48 8B FA 48 8B D9 33 F6 48 8D 4C 24"),
                Item(name="RemoveItem",
                     pattern="48 89 5C 24 08 57 48 83 EC 40 49 8B F8 4C 8B C2 48 8D 54 24 20 E8 ? ? ? ? 44 8B 47"),
                Item(name="ProcessGarment",
                     pattern="48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 57 48 83 EC ? 49 8B D9 49 8B F8 48 8B F2 48 8B E9 45 33 C9 48 8D 48 ? 41 B0 ? 33 D2"),
                Item(name="ProcessSkinnedMesh",
                     pattern="48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 57 41 56 41 57 48 81 EC ? ? ? ? 8B FA 4C 8B F1 49 8B 11 48 8D 48 ? 48 83 C2 ? 4D 8B F8"),
                Item(name="ProcessMorphedMesh",
                     pattern="48 8B C4 48 89 58 ? 48 89 70 ? 48 89 78 ? 4C 89 60 ? 55 41 56 41 57 48 8D 68 ? 48 81 EC ? ? ? ? 49 8B 19 4C 8B F9 4D 8B E0 8B FA 48 8B 4B"),
                Item(name="OnGameDetach",
                     pattern="48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B F1 E8 ? ? ? ? 48 8D 8E ? ? ? ? E8"),
            ]),
            Group(name="GarmentAssemblerState", functions=[
                Item(name="AddItem",
                     pattern="4C 8B DC 49 89 5B ? 49 89 6B ? 49 89 73 ? 57 48 83 EC ? 48 8B 02 48 8B F2",
                     expected=2,
                     index=0),
                Item(name="AddCustomItem",
                     pattern="4C 8B DC 49 89 5B ? 49 89 6B ? 49 89 73 ? 57 48 83 EC ? 48 8B 02 48 8B F2",
                     expected=2,
                     index=1),
                Item(name="ChangeItem",
                     pattern="48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 48 89 78 ? 41 56 48 83 EC ? 44 8B 42 ? 48 8B FA",
                     expected=2,
                     index=0),
                Item(name="ChangeCustomItem",
                     pattern="48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 48 89 78 ? 41 56 48 83 EC ? 44 8B 42 ? 48 8B FA",
                     expected=2,
                     index=1),
            ]),
            Group(name="InkSpawner", functions=[
                Item(name="FinishAsyncSpawn",
                     pattern="48 89 5C 24 ? 57 48 83 EC ? 48 8B F9 48 8B DA 48 8B 49 ? 48 81 C1 ? ? ? ? E8 ? ? ? ? 48 8B 13"),
            ]),
            Group(name="InkWidgetLibrary", functions=[
                Item(name="AsyncSpawnFromExternal",
                     pattern="48 89 5C 24 ? 57 48 83 EC ? 48 8B FA 49 8B D9 48 8D 54 24 ? E8 ? ? ? ? 48 8B 4C 24 ? 48 85 C9 74 ? 45 33 C9 4C 8B C3 48 8B D7 E8"),
                Item(name="AsyncSpawnFromLocal",
                     pattern="48 83 EC ? 4C 8B D2 49 8B D0 E8 ? ? ? ? 48 85 C0 74 ? 45 8A C1 49 8B D2 48 8B C8 E8 ? ? ? ? B0 01 48 83 C4 ? C3"),
                Item(name="SpawnFromExternal",
                     pattern="48 89 5C 24 ? 57 48 83 EC ? 48 8B FA 49 8B D9 48 8D 54 24 ? E8 ? ? ? ? 48 8B 4C 24 ? 48 85 C9 74 ? 4C 8B C3 48 8B D7 E8"),
                Item(name="SpawnFromLocal",
                     pattern="40 53 48 83 EC 20 48 8B DA 49 8B D0 E8 ? ? ? ? 48 85 C0 74 ? 48 8B D3 48 8B C8 E8 ? ? ? ? 48 8B C3"),
            ]),
            Group(name="ItemFactoryAppearanceChangeRequest", functions=[
                Item(name="LoadTemplate",
                     pattern="48 8B C4 48 89 58 ? 48 89 70 ? 48 89 78 ? 55 48 8D 68 ? 48 81 EC ? ? ? ? 33 FF 48 8D B1 ? ? ? ? 48 8B D9 48"),
                Item(name="LoadAppearance",
                     pattern="48 8B C4 48 89 58 ? 48 89 70 ? 48 89 78 ? 4C 89 70 ? 55 48 8B EC 48 83 EC ? 48 8B F1 33 FF 48 8B 49 ? 48 89 4D"),
            ]),
            Group(name="ItemFactoryRequest", functions=[
                Item(name="LoadAppearance",
                     pattern="48 89 5C 24 ? 55 56 57 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B D9 33 FF 48 8B 49 ? 48 89 4C 24 ? 48 8B 43 ? 48 89 44 24"),
            ]),
            Group(name="JobHandle", functions=[
                Item(name="Wait",
                     pattern="48 8B 11 41 83 C9 FF 48 8B 0D ? ? ? ? 45 33 C0 E9"),
            ]),
            Group(name="JournalManager", functions=[
                Item(name="LoadJournal",
                     pattern="48 8B C4 48 89 58 ? 48 89 70 ? 48 89 78 ? 55 48 8D 68 ? 48 81 EC ? ? ? ? 48 8B F2 48 8B F9 E8 ? ? ? ? 48 8D 57"),
                Item(name="TrackQuest",
                     pattern="40 55 53 56 57 41 54 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B F1 33 FF 48 8B CA 44 0F B6 FF 4C 8B F2 E8"),
            ]),
            Group(name="JournalRootFolderEntry", functions=[
                Item(name="Initialize",
                     pattern="48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 4D 8B C1 49 8B F1 48 8B DA 48 8B F9 E8 ? ? ? ? 4C 8B C6 48 8B D3"),
            ]),
            Group(name="JournalTree", functions=[
                Item(name="ProcessJournalIndex",
                     pattern="48 89 5C 24 ? 48 89 74 24 ? 55 57 41 54 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 45 33 E4 C7 44 24 ? FF FF FF FF"),
            ]),
            Group(name="Localization", functions=[
                Item(name="LoadOnScreens",
                     pattern="48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 48 8B EC 48 83 EC ? 0F 57 C0 48 8B D9 F3 0F 7F 45 ? 48 85 D2 0F 84"),
                Item(name="LoadSubtitles",
                     pattern="48 89 5C 24 ? 55 48 8B EC 48 83 EC ? 0F 57 C0 48 8B D9 F3 0F 7F 45 ? 48 85 D2 0F 84 ? ? ? ? 48 8B 0D"),
                Item(name="LoadVoiceOvers",
                     pattern="48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 48 89 54 24 ? 55 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC"),
                Item(name="LoadLipsyncs",
                     pattern="48 89 5C 24 ? 57 48 83 EC ? 8A DA 48 8B F9 48 8B 15 ? ? ? ? 48 8D 4C 24 ? 48 81 C2 ? ? ? ? E8 ? ? ? ? 48 8D 54 24"),
            ]),
            Group(name="MappinSystem", functions=[
                Item(name="GetMappinData",
                     pattern="48 8B 41 ? 48 85 C0 74 ? 44 8B 40 4C 48 8B 48 40 49 C1 E0 05 4C 03 C1 49 3B C8 74",
                     expected=2,
                     index=0),
                Item(name="GetPoiData",
                     pattern="48 8B 41 ? 48 85 C0 74 ? 44 8B 40 4C 48 8B 48 40 49 C1 E0 05 4C 03 C1 49 3B C8 74",
                     expected=2,
                     index=1),
                Item(name="OnStreamingWorldLoaded",
                     pattern="48 89 5C 24 ? 55 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 82 ? ? ? ? 48 8B D9"),
            ]),
            Group(name="MeshAppearance", functions=[
                Item(name="LoadMaterialSetupAsync",
                     pattern="48 89 5C 24 ? 55 56 57 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 65 48 8B 04 25 ? ? ? ? 48 8B D9 B9 ? ? ? ? 0F 57 C0"),
            ]),
            Group(name="QuestLoader", functions=[
                Item(name="ProcessPhaseResource",
                     pattern="48 89 5C 24 ? 48 89 74 24 ? 48 89 54 24 ? 55 57 41 56 48 8B EC 48 83 EC ? 48 8D 59 ? 48 8B F9 48 8B CB 4D 8B F0 E8"),
            ]),
            Group(name="ResourceDepot", functions=[
                Item(name="InitializeArchives",
                     pattern="48 89 5C 24 ? 55 56 57 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B F1 48 8D 91 ? ? ? ? 48 8D 4D ? E8 ? ? ? ? 48 8D 7E"),
                Item(name="LoadArchives",
                     pattern="4C 89 4C 24 ? 48 89 4C 24 ? 55 53 56 57 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 45 8B 78 ? 41 83 CE FF"),
            ]),
            Group(name="ResourceLoader", functions=[
                Item(name="OnUpdate",
                     pattern="48 89 5C 24 ? 48 89 74 24 ? 55 57 41 54 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 33 F6 4C 8B F1"),
            ]),
            Group(name="ResourcePath", functions=[
                Item(name="Create",
                     pattern="40 53 48 81 EC ? ? ? ? 83 7A 08 00 48 8B D9 74 ? F2 0F 10 02 48 8D 4C 24 ? 8B 42 08 48 8D 54 24 ? F2 0F 11 44 24"),
            ]),
            Group(name="StreamingSector", functions=[
                Item(name="OnReady",
                     pattern="48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B 91 ? ? ? ? 33 FF 48 8B D9"),
            ]),
            Group(name="StreamingWorld", functions=[
                Item(name="OnLoad",
                     pattern="48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC ? 4C 8B F2 48 8B F9 E8 ? ? ? ? 41 F6 46 08 02 4C 8D A7"),
            ]),
#             Group(name="TagList", functions=[
#                 Item(name="MergeWith",
#                      pattern="48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 48 8B 1A 48 8B F1 8B 42 0C 48 8D 3C C3 48 3B DF 74"),
#             ]),
            Group(name="TPPRepresentationComponent", functions=[
                Item(name="OnAttach",
                     pattern="48 89 5C 24 ? 48 89 74 24 ? 55 57 41 56 48 8B EC 48 83 EC ? 66 83 89 ? ? ? ? 02 48 8B DA 48 8B F1 E8"),
                Item(name="OnItemEquipped",
                     pattern="48 89 5C 24 ? 48 89 74 24 ? 55 57 41 54 41 56 41 57 48 8B EC 48 81 EC ? ? ? ? 48 8B F1 49 8B D8 49 8B C8 48 8B FA E8"),
                Item(name="OnItemUnequipped",
                     pattern="48 8B C4 48 89 58 ? 48 89 70 ? 4C 89 40 ? 48 89 50 ? 57 48 83 EC ? 48 8B F1 49 8B D8 49 8B C8 48 8B FA E8"),
                Item(name="RegisterAffectedItem",
                     pattern="48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D B9 ? ? ? ? 48 8B E9 48 8B CF 49 8B F0 48 8B DA E8 ? ? ? ? 0F 57 C0 48 89 5C 24 ? 48 8B D6 48 8D 4C 24"),
                Item(name="IsAffectedSlot",
                     pattern="48 89 4C 24 ? 48 8D 15 ? ? ? ? 48 8B 02 3B C1 75 ? 48 C1 E8 ? 3A 44 24 ? 74"),
            ]),
#             Group(name="TransactionSystem", functions=[
#                 Item(name="IsSlotSpawning",
#                      pattern="",
#                      expected=3,
#                      index=1),
#             ]),
            Group(name="TweakDB", functions=[
                Item(name="Load",
                     pattern="48 89 5C 24 ? 48 89 7C 24 ? 55 48 8B EC 48 ? EC ? ? ? ? 48 8B F9 48 8B DA 48 8B 0D"),
            ]),
        ]),
    ]


# Defines base output dir
def output_dir():
    cwd = Path(__file__).resolve().parent
    return cwd.parent.parent  # 2 levels up


scan(patterns(), output_dir(), __file__)
