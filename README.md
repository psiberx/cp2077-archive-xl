# ArchiveXL

Archive Extension Loader allows you to expand game resources that are currently not suitable for modifications without conflicts.

## Installation

Just extract the release archive into your game folder.
Once installed, the loader is ready to use, no additional action is required.

## Modding

### Features

With the loader you can:

- Load custom entity factories
- Add localized texts that can be used in scripts, other resources and TweakDB

The loader doesn't modify original files such as `factories.csv` and `onscreens.json` and doesn't produce new archives. Everything is done at runtime.

### How to load custom resources

The workflow is pretty simple:

1. Create files in original REDengine formats and add them to the archive as usual.
2. Add config file that tells the loader which resources from the archive to load in addition to the original game files, and include it in the mod along with the archive.

So you should have two files that will be installed in the `mod` directory:

```
<Cyberpunk 2077>\archive\pc\mod\bestmod.archive
<Cyberpunk 2077>\archive\pc\mod\bestmod.archive.xl
```

In this example, the config file is named after the archive for convenience only.
You can name it as you want.

### Create resources

As stated earlier, you should simply use the original REDengine formats for the new files.

#### Factories

`base\gameplay\factories\bestmod\clothing.csv.json`:

```json
{
  "Chunks": {
    "0": {
      "Type": "C2dArray",
      "ParentIndex": -1,
      "Properties": {
        "cookingPlatform": [
          "PLATFORM_PC"
        ],
        "headers": [
          "name",
          "path",
          "preload"
        ],
        "data": [
          [
            "player_face_item",
            "base\\gameplay\\items\\equipment\\face\\player_face_item.ent",
            "true"
          ],
          [
            "bestmod_face_item_001",
            "base\\gameplay\\items\\equipment\\face\\bestmod_face_item_001.ent",
            "true"
          ]
        ]
      }
    }
  }
}
```

#### Localization

`base\localization\en-us\bestmod.json.json`:

```json
{
  "Extension": ".json",
  "Chunks": {
    "0": {
      "Type": "JsonResource",
      "ParentIndex": -1,
      "Properties": {
        "cookingPlatform": [
          "PLATFORM_PC"
        ],
        "root": 2
      }
    },
    "1": {
      "Type": "localizationPersistenceOnScreenEntries",
      "ParentIndex": -1,
      "Properties": {
        "entries": [
          {
            "primaryKey": 300001,
            "femaleVariant": "Samurai Mask & Aviators"
          },
          {
            "primaryKey": 300002,
            "secondaryKey": "Some-Human-Readable-Key",
            "femaleVariant": "Lorem ipsum dolor sit amet",
            "maleVariant": "Sed ut perspiciatis unde omnis"
          }
        ]
      }
    }
  }
}
```

The `primaryKey` is required, while the `secondaryKey` is optional and can be omitted. If you don't need gender specific translations you should set `femaleVariant` only and it'll be used for all genders.

### Write loader configuration

The config file is a YAML file with a `.xl` extension:

```yaml
factories:
  - base\gameplay\factories\bestmod\clothing.csv
  - base\gameplay\factories\bestmod\weapons.csv
localization:
  onscreens:
    en-us:
      - base\localization\en-us\bestmod.json
    de-de:
      - base\localization\de-de\bestmod-1.json
      - base\localization\de-de\bestmod-2.json
```

In all cases where you only have one resource to load, you can use the short syntax:

```yaml
factories: base\gameplay\factories\bestmod\items.csv
localization:
  onscreens:
    en-us: base\localization\en-us\bestmod.json
    de-de: base\localization\de-de\bestmod.json
```

The first language in the list is also a fallback and will be used when the user enables a language that is not supported by the mod.

Language codes used by the game:

| Code    | Language | Code    | Language |
| :---    | :---     | :---    | :---     |
| `pl-pl` | Polish | `ru-ru` | Russian |
| `en-us` | English | `pt-br` | Brazilian Portuguese |
| `es-es` | Spanish | `jp-jp` | Japanese |
| `fr-fr` | French | `zh-tw` | Traditional Chinese |
| `it-it` | Italian | `ar-ar` | Arabic |
| `de-de` | German | `cz-cz` | Czech |
| `es-mx` | Latin American Spanish | `hu-hu` | Hungarian |
| `kr-kr` | Korean | `tr-tr` | Turkish |
| `zh-cn` | Simplified Chinese | `th-th` | Thai |

## Next steps

- Generate TweakDB records from simple definitions (might be implemented as a part of TweakDBext)
- Add localized subtitles
- Collect feedback
