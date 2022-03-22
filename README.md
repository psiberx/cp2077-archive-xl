# ArchiveXL

ArchiveXL is a modding tool that allows you to load game resources without touching original game files,
thus allowing multiple mods to expand same resources without conflicts.

With the loader you can:

- Load custom entity factories (necessary for item additions)
- Add localization texts that can be used in scripts, resources and TweakDB
- Edit existing localizations without overwriting original resources

## Getting Started

### Prerequisites

- [RED4ext](https://docs.red4ext.com/getting-started/installing-red4ext) 1.4.0+

### Installation

1. Download [the latest release](https://github.com/psiberx/cp2077-archive-xl/releases) archive
2. Extract the archive into the Cyberpunk 2077 installation directory

## Creating Mods

### Basic concept

The loader doesn't modify original files such as `factories.csv` and `onscreens.json`
and doesn't produce any new archives.
Instead it loads your own resources and merges them in at runtime.

The workflow is pretty simple:

1. Create files in original REDengine formats and add them to the archive as usual.
2. Add config file that tells the loader which resources from the archive to load in
3. addition to the original game files, and include it in the mod along with the archive.

So you should have two files that will be installed in the `mod` directory:

```
<Cyberpunk 2077>\archive\pc\mod\bestmod.archive
<Cyberpunk 2077>\archive\pc\mod\bestmod.archive.xl
```

In this example, the config file is named after the archive for convenience only.
You can name it as you want.

### Create resources

As stated earlier, you should simply use the original REDengine formats for the new files.

#### Entity Factory

`mod\factories\clothing.csv`:

```json
{
  "Chunks": {
    "0": {
      "Type": "C2dArray",
      "ParentIndex": -1,
      "Properties": {
        "cookingPlatform": ["PLATFORM_PC"],
        "headers": ["name", "path", "preload"],
        "data": [
          [
            "mod_face_item_001",
            "mod\\items\\face_item_001.ent",
            "true"
          ],
          [
            "mod_face_item_001_app",
            "mod\\items\\face_item_001.app",
            "false"
          ]
        ]
      }
    }
  }
}
```

Despite the name it can contain both entities `.ent` and appearance `.app` resources.

#### Localization

`mod\localization\en-us.json`:

```json
{
  "Extension": ".json",
  "Chunks": {
    "0": {
      "Type": "JsonResource",
      "ParentIndex": -1,
      "Properties": {
        "cookingPlatform": ["PLATFORM_PC"],
        "root": 2
      }
    },
    "1": {
      "Type": "localizationPersistenceOnScreenEntries",
      "ParentIndex": -1,
      "Properties": {
        "entries": [
          {
            "secondaryKey": "Mod-Face-Item-01-Name",
            "femaleVariant": "Samurai Mask & Aviators"
          },
          {
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



The `primaryKey` is required if you want to edit original translations.
When creating you own localization it's recommended to always use the `secondaryKey`.
If you don't need gender specific translations you should set `femaleVariant` only and
it'll be used for all genders.

### Write loader configuration

The config file is a YAML file with a `.xl` extension:

```yaml
factories:
  - mod\factories\clothing.csv
  - mod\factories\weapons.csv
localization:
  onscreens:
    en-us: mod\localization\en-us.json
    de-de: mod\localization\de-de.json
```

In all cases where you only have one resource to load, you can use the short syntax:

```yaml
factories: base\factories\items.csv
```

The first language in the list is also a fallback and will be used when the user enables a language
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
