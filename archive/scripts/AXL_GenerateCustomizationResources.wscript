import * as Logger from 'Logger.wscript'
import * as TypeHelper from 'TypeHelper.wscript'

const resources = [
  'archive_xl\\characters\\head\\player_base_heads\\appearances\\head\\hel_pwa_lashes.app.json',
  'archive_xl\\characters\\head\\player_base_heads\\appearances\\head\\hel_pma_lashes.app.json',
  'archive_xl\\characters\\head\\player_base_heads\\player_female_average\\heb_pwa_brows_patch.morphtarget.json',
  'archive_xl\\characters\\head\\player_base_heads\\player_female_average\\h0_000_pwa_c__basehead\\heb_pwa_brows_patch.mesh.json',
  'archive_xl\\characters\\head\\player_base_heads\\player_female_average\\h0_000_pwa_c__basehead\\he_000_pwa_c__basehead_patch.mesh.json',
  'archive_xl\\characters\\head\\player_base_heads\\player_man_average\\heb_pma_brows_patch.morphtarget.json',
  'archive_xl\\characters\\head\\player_base_heads\\player_man_average\\h0_000_pma_c__basehead\\heb_pma_brows_patch.mesh.json',
  'archive_xl\\characters\\head\\player_base_heads\\player_man_average\\h0_000_pma_c__basehead\\he_000_pma_c__basehead_patch.mesh.json',
]

const templates = [
  {
    path: 'archive_xl\\characters\\head\\player_base_heads\\appearances\\head\\eyebrows\\heb_pwa_brows__01.app.json',
    patterns: [/(?<=heb_pwa_brows__)(\d+)/g],
    range: [1, 13],
  },
  {
    path: 'archive_xl\\characters\\head\\player_base_heads\\player_female_average\\heb_pwa_brows__01.morphtarget.json',
    patterns: [/(?<=heb_pwa_brows__)(\d+)/g],
    range: [1, 13],
  },
  {
    path: 'archive_xl\\characters\\head\\player_base_heads\\player_female_average\\h0_000_pwa_c__basehead\\heb_pwa_brows__01.mesh.json',
    patterns: [/(?<=heb_pwa_brows__)(\d+)/g, /(?<=_d|_ds|_n?)(\d+)(?=\.xbm)/g],
    range: [1, 13],
  },
  {
    path: 'archive_xl\\characters\\head\\player_base_heads\\appearances\\head\\eyebrows\\heb_pma_brows__01.app.json',
    patterns: [/(?<=heb_pma_brows__)(\d+)/g],
    range: [1, 13],
  },
  {
    path: 'archive_xl\\characters\\head\\player_base_heads\\player_man_average\\heb_pma_brows__01.morphtarget.json',
    patterns: [/(?<=heb_pma_brows__)(\d+)/g],
    range: [1, 13],
  },
  {
    path: 'archive_xl\\characters\\head\\player_base_heads\\player_man_average\\h0_000_pma_c__basehead\\heb_pma_brows__01.mesh.json',
    patterns: [/(?<=heb_pma_brows__)(\d+)/g, /(?<=_d|_ds|_n?)(\d+)(?=\.xbm)/g],
    range: [1, 13],
  },
]

for (let jsonPath of resources) {
  let resourcePath = jsonPath.replace(/.json$/, '')
  let resourceJson = wkit.LoadRawJsonFromProject(jsonPath, 'json')

  wkit.SaveToProject(resourcePath, wkit.JsonToCR2W(resourceJson))
}

for (let template of templates) {
  template.json = wkit.LoadRawJsonFromProject(template.path, 'json')

  if (template.patterns && template.range) {
    for (let i = template.range[0]; i <= template.range[1]; ++i) {
      let resourceNumber = i.toString().padStart(2, '0')
      let resourcePath = template.path.replace(/.json$/, '')
      let resourceJson = template.json

      for (let pattern of template.patterns) {
        resourcePath = resourcePath.replaceAll(pattern, resourceNumber)
        resourceJson = resourceJson.replaceAll(pattern, resourceNumber)
      }

      wkit.SaveToProject(resourcePath, wkit.JsonToCR2W(resourceJson))
    }
  }
}
