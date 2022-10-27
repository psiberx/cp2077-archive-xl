# ArchiveXL

ArchiveXL is a modding tool that allows you to load game resources without touching original game files,
thus allowing multiple mods to expand same resources without conflicts.

With the loader you can:

- Load custom entity factories (necessary for item additions)
- Add localization texts that can be used in scripts, resources and TweakDB
- Edit existing localization texts without overwriting original resources
- Override submeshes visibility of entity parts
- Add visual tags to a clothing item
- Spawn widgets from any library without registering dependencies

## Getting Started

### Prerequisites

- [RED4ext](https://docs.red4ext.com/getting-started/installing-red4ext) 1.8.0+

### Installation

1. Download [the latest release](https://github.com/psiberx/cp2077-archive-xl/releases) archive
2. Extract the archive into the Cyberpunk 2077 installation directory

## Documentation

### Extending resources

The loader doesn't modify original files such as `factories.csv` and `onscreens.json` and doesn't produce new archives,
instead it merges your own resources with the originals at runtime.

The workflow is pretty simple:

1. Create CR2W resources with your content and add them to the archive as usual
2. Create a loader config that specifies which resources from your archive should be merged with the original game files
3. Include the loader config in the mod along with the archive

#### Entity Factory

`mymod\factories\clothing.csv`:

```json
{
  "Header": {
    "WKitJsonVersion": "0.0.3",
    "GameVersion": 1600,
    "DataType": "CR2W"
  },
  "Data": {
    "Version": 195,
    "BuildVersion": 0,
    "RootChunk": {
      "$type": "C2dArray",
      "compiledData": [
        [
          "mymod_item",
          "mymod\\items\\clothing\\item.ent",
          "true"
        ]
      ],
      "compiledHeaders": [
        "name",
        "path",
        "preload"
      ],
      "cookingPlatform": "PLATFORM_PC"
    },
    "EmbeddedFiles": []
  }
}
```

Despite the name the factory can contain both entities (`.ent`) and appearance (`.app`) resources.

#### Localization

`mymod\localization\en-us.json`:

```json
{
  "Header": {
    "WKitJsonVersion": "0.0.3",
    "GameVersion": 1600,
    "DataType": "CR2W"
  },
  "Data": {
    "Version": 195,
    "BuildVersion": 0,
    "RootChunk": {
      "$type": "JsonResource",
      "cookingPlatform": "PLATFORM_PC",
      "root": {
        "HandleId": "0",
        "Data": {
          "$type": "localizationPersistenceOnScreenEntries",
          "entries": [
            {
              "$type": "localizationPersistenceOnScreenEntry",
              "secondaryKey": "MyMod-Item-Name",
              "femaleVariant": "Samurai Mask & Aviators"
            }
          ]
        }
      }
    },
    "EmbeddedFiles": []
  }
}
```

When creating you own localization it's recommended to always use the `secondaryKey`.
If you don't need gender specific translations you should set `femaleVariant` only and it'll be used for all genders.

If you want to edit original translations you must use the `primaryKey`.

#### Loader configuration

The config file is a YAML file with a `.xl` extension that must be placed in the same directory with the archive:

```
<Cyberpunk 2077>\archive\pc\mod\mymod.archive
<Cyberpunk 2077>\archive\pc\mod\mymod.archive.xl
```

Or if you use REDmod packaging:

```
<Cyberpunk 2077>\mods\betsmod\archives\mymod.archive
<Cyberpunk 2077>\mods\betsmod\archives\mymod.archive.xl
```

In this example, the config file is named after the archive for convenience. But you can name it as you want.

In the config file you have to specify factories and/or localization resources to load:

```yaml
factories:
  - mymod\factories\clothing.csv
  - mymod\factories\weapons.csv
localization:
  onscreens:
    en-us: mymod\localization\en-us.json
    de-de: mymod\localization\de-de.json
```

The first language in the list is also a fallback language and will be used when the user enables a language
that is not supported by the mod.

Language codes used by the game:

| Code    | Language               | Code    | Language             |
|:--------|:-----------------------|:--------|:---------------------|
| `pl-pl` | Polish                 | `ru-ru` | Russian              |
| `en-us` | English                | `pt-br` | Brazilian Portuguese |
| `es-es` | Spanish                | `jp-jp` | Japanese             |
| `fr-fr` | French                 | `zh-tw` | Traditional Chinese  |
| `it-it` | Italian                | `ar-ar` | Arabic               |
| `de-de` | German                 | `cz-cz` | Czech                |
| `es-mx` | Latin American Spanish | `hu-hu` | Hungarian            |
| `kr-kr` | Korean                 | `tr-tr` | Turkish              |
| `zh-cn` | Simplified Chinese     | `th-th` | Thai                 |

### Overriding submeshes

To hide submeshes of the owner entity, such as body parts, in your appearance definition you must
add a `partsOverrides` entry with a blank `partResource` and add `componentOverrides` using entity's component names.

It's similar to how you override parts of your own meshes, but instead you use the component names of the entity
that wears your item, such as `t0_000_pma_base__full` or `l0_000_pwa_base__cs_flat`.

### Adding visual tags

To apply visual tags to an item you must add them to `visualTags` property of your appearance definition.

Known visual tags defined by the game:

| Tag             | Effect                                                                      |
|:----------------|:----------------------------------------------------------------------------|
| `hide_H1`       | Hides an item in the `Head` slot.                                           |
| `hide_F1`       | Hides an item in the `Eyes` slot.                                           |
| `hide_T1`       | Hides an item in the `Chest` slot.                                          |
| `hide_T2`       | Hides an item in the `Torso` slot.                                          |
| `hide_L1`       | Hides an item in the `Legs` slot.                                           |
| `hide_S1`       | Hides an item in the `Feet` slot.                                           |
| `hide_T1part`   | Controls the partial suffix (`&Full`/`&Part`) when applied to `Torso` item. |
| `hide_Hair`     | Hides hair.                                                                 |
| `hide_Genitals` | Hides genitals in uncensored mode and underware in censored mode.           |

Custom visual tags defined by the loader:

| Tag           | Effect                |
|:--------------|:----------------------|
| `hide_Head`   | Hides head.           |
| `hide_Torso`  | Hides torso.          |
| `hide_Arms`   | Hides head.           |
| `hide_Thighs` | Hides thighs.         |
| `hide_Calves` | Hides calves.         |
| `hide_Ankles` | Hides ankles.         |
| `hide_Feet`   | Hides feet.           |
| `hide_Legs`   | Hides the whole legs. |

### Spawning widgets

With the loader you can spawn any widget without the need to register it as dependency.

```swift
controller.SpawnFromExternal(parentWidget, r"base\\gameplay\\gui\\fullscreen\\settings\\settings_main.inkwidget", n"settingsSelectorBool");
```
