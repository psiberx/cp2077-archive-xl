import * as Logger from 'Logger.wscript'
import { CName } from 'TypeHelper.wscript'
import * as TypeHelper from 'TypeHelper.wscript'

const logInfo = false
const logWarnings = true

const generateFixedResources = false
const generateExtensionJson = true

const hairMaMainMeshes = [
  'base\\characters\\common\\hair\\hh_006_ma__demo\\hh_006_ma__demo.mesh',
  'base\\characters\\common\\hair\\hh_007_ma__demo\\hh_007_ma__demo.mesh',
  'base\\characters\\common\\hair\\hh_007_ma__demo\\hh_007_pma__demo_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_008_ma__demo\\hh_008_ma__sk8t_or_die.mesh',
  'base\\characters\\common\\hair\\hh_008_ma__demo\\hh_008_pma__sk8t_or_die_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_011_wa__demo\\hh_011_ma__demo.mesh',
  'base\\characters\\common\\hair\\hh_025_ma__pompadour\\hh_025_ma__pompadour.mesh',
  'base\\characters\\common\\hair\\hh_025_ma__pompadour\\hh_025_pma__pompadour_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_026_ma__rattail\\hh_026_ma__rat_tail.mesh',
  'base\\characters\\common\\hair\\hh_026_ma__rattail\\hh_026_pma__rat_tail_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_027_ma__scavenger\\hh_027_ma__scavenger.mesh',
  'base\\characters\\common\\hair\\hh_028_wa__corpo_bun\\hh_028_ma__corpo_bun.mesh',
  'base\\characters\\common\\hair\\hh_028_wa__corpo_bun\\hh_028_pma__corpo_bun_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_030_ma__punk_idol\\hh_030_ma__punk_idol.mesh',
  'base\\characters\\common\\hair\\hh_031_mb__morgan_blackhand\\hh_031_ma__morgan_blackhand.mesh',
  'base\\characters\\common\\hair\\hh_032_ma__ripper_doc\\hh_032_ma__ripper_doc_common.mesh',
  'base\\characters\\common\\hair\\hh_032_ma__ripper_doc\\hh_032_pma__ripper_doc_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_035_ma__mohawk_tall\\hh_035_ma__mohawk_tall.mesh',
  'base\\characters\\common\\hair\\hh_035_ma__mohawk_tall\\hh_035_pma__mohawk_tall_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_036_ma__high_tight\\hh_036_ma__high_tight.mesh',
  'base\\characters\\common\\hair\\hh_036_ma__high_tight\\hh_036_pma__high_tight_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_037_ma__gungho\\hh_037_ma__gungho.mesh',
  'base\\characters\\common\\hair\\hh_037_ma__gungho\\hh_037_pma__gungho_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_039_wa__punk_shaved\\hh_039_ma__punk_shaved.mesh',
  'base\\characters\\common\\hair\\hh_039_wa__punk_shaved\\hh_039_pma__punk_shaved_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_040_wa__pixie_bob\\hh_040_ma__pixie_bob.mesh',
  'base\\characters\\common\\hair\\hh_045_ma__short_spiked\\hh_045_ma__short_spiked.mesh',
  'base\\characters\\common\\hair\\hh_045_ma__short_spiked\\hh_045_pma__short_spiked_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_047_wa__swirl_pomp\\hh_047_ma__swirl_pomp.mesh',
  'base\\characters\\common\\hair\\hh_047_wa__swirl_pomp\\hh_047_pma__swirl_pomp_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_048_ma__dual_braids\\hh_048_ma__dual_braids.mesh',
  'base\\characters\\common\\hair\\hh_049_ma__thiago\\hh_049_ma__thiago_common.mesh',
  'base\\characters\\common\\hair\\hh_051_wa__judy\\hh_051_ma__judy_common.mesh',
  'base\\characters\\common\\hair\\hh_051_wa__judy\\hh_051_pma__judy_common_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_053_ma__kerry_eurodyne\\hh_053_ma__kerry_eurodyne_common.mesh',
  'base\\characters\\common\\hair\\hh_053_ma__kerry_eurodyne\\hh_053_pma__kerry_eurodyne_common_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_058_wa__voodoo_01\\hh_058_ma__voodoo_01.mesh',
  'base\\characters\\common\\hair\\hh_059_wa__voodoo_02\\hh_059_ma__voodoo_02.mesh',
  'base\\characters\\common\\hair\\hh_059_wa__voodoo_02\\hh_059_pma__voodoo_02_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_060_wa__voodoo_03\\hh_060_ma__voodoo_03.mesh',
  'base\\characters\\common\\hair\\hh_060_wa__voodoo_03\\hh_060_pma__voodoo_03_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_061_ma__midlength_wavy\\hh_061_ma__midlength_wavy.mesh',
  'base\\characters\\common\\hair\\hh_062_ma__slick_back\\hh_062_ma__slick_back.mesh',
  'base\\characters\\common\\hair\\hh_065_wa__afro_knots\\hh_065_ma__afro_knots.mesh',
  'base\\characters\\common\\hair\\hh_065_wa__afro_knots\\hh_065_pma__afro_knots_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_068_wba__animals_03\\hh_068_ma__animals_03.mesh',
  'base\\characters\\common\\hair\\hh_073_ma__nomad_01\\hh_073_ma__nomad_01.mesh',
  'base\\characters\\common\\hair\\hh_075_ma__peralez\\hh_075_ma__peralez_common.mesh',
  'base\\characters\\common\\hair\\hh_082_ma__afro_bun\\hh_082_ma__afro_bun.mesh',
  'base\\characters\\common\\hair\\hh_082_ma__afro_bun\\hh_082_ma__afro_bun_bun.mesh',
  'base\\characters\\common\\hair\\hh_082_ma__afro_bun\\hh_082_pma__afro_bun_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_085_ma__takemura\\hh_085_ma__takemura_common.mesh',
  'base\\characters\\common\\hair\\hh_089_ma__thompson\\hh_089_ma__thompson_common.mesh',
  'base\\characters\\common\\hair\\hh_089_ma__thompson\\hh_089_pma__thompson_common_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_093_mba__sumo\\hh_093_ma__sumo.mesh',
  'base\\characters\\common\\hair\\hh_093_mba__sumo\\hh_093_ma__sumo_knot.mesh',
  'base\\characters\\common\\hair\\hh_094_ma__saul\\hh_094_ma__saul_common.mesh',
  'base\\characters\\common\\hair\\hh_103_ma__maelstrom_spikes\\hh_103_ma__common_spikes.mesh',
  'base\\characters\\common\\hair\\hh_112_ma__kicinski\\hh_112_ma__kicinski_player.mesh',
  'base\\characters\\common\\hair\\hh_113_ma__iwinski\\hh_113_ma__iwinski_common.mesh',
  'base\\characters\\common\\hair\\hh_120_ma__arasaka_bun\\hh_120_ma__arasaka_bun_common_01.mesh',
  'base\\characters\\common\\hair\\hh_120_ma__arasaka_bun\\hh_120_ma__arasaka_bun_knot_02.mesh',
  'base\\characters\\common\\hair\\hh_120_ma__arasaka_bun\\hh_120_pma__arasaka_bun_01_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_122_ma__roy\\hh_122_ma__roy.mesh',
  'base\\characters\\common\\hair\\hh_140_ma__short_afro\\hh_140_ma__short_afro.mesh',
  'base\\characters\\common\\hair\\hh_140_ma__short_afro\\hh_140_pma__short_afro_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_141_ma__afro\\hh_141_ma__afro.mesh',
  'base\\characters\\common\\hair\\hh_142_ma__afrohawk\\hh_142_pma__afrohawk.mesh',
  'base\\characters\\common\\hair\\hh_142_ma__afrohawk\\hh_142_pma__afrohawk_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_143_ma__flat_top\\hh_143_ma__flat_top.mesh',
  'base\\characters\\common\\hair\\hh_143_ma__flat_top\\hh_143_pma__flat_top_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_144_wa__afro_braid_bun\\hh_144_ma__afro_braid_bun.mesh',
  'base\\characters\\common\\hair\\hh_144_wa__afro_braid_bun\\hh_144_pma__afro_braid_bun_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_145_ma__v_short\\hh_145_ma__v_short.mesh',
  'base\\characters\\common\\hair\\hh_145_ma__v_short\\hh_145_pma__v_short.mesh',
  'base\\characters\\common\\hair\\hh_146_ma__dread_undercut\\hh_146_ma__dread_undercut.mesh',
  'base\\characters\\common\\hair\\hh_146_ma__dread_undercut\\hh_146_pma__dread_undercut_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_999_ma__buzz_cap\\hh_999_ma__buzz_cap.mesh',
  'base\\characters\\common\\hair\\hh_999_ma__buzz_cap\\hh_999_pma__buzz_cap_cyberware_01.mesh',
]

const hairMaHatMeshes = [
  'base\\characters\\common\\hair\\h1_003_wa_hat__beanie_hh_01\\h1_003_ma_hat__beanie_hh_01.mesh',
  'base\\characters\\common\\hair\\h1_003_wa_hat__beanie_hh_02\\h1_003_ma_hat__beanie_hh_02.mesh',
  'base\\characters\\common\\hair\\h1_003_wa_hat__beanie_hh_03\\h1_003_ma_hat__beanie_hh_03.mesh',
  'base\\characters\\common\\hair\\h1_003_wa_hat__beanie_hh_04\\h1_003_ma_hat__beanie_hh_04.mesh',
  'base\\characters\\common\\hair\\h1_011_ma_hat__baseball_back_hh_01\\h1_011_ma_hat__baseball_back_hh_01.mesh',
  'base\\characters\\common\\hair\\h1_011_ma_hat__baseball_back_hh_02\\h1_011_ma_hat__baseball_back_hh_02.mesh',
  'base\\characters\\common\\hair\\h1_011_ma_hat__baseball_back_hh_03\\h1_011_ma_hat__baseball_back_hh_03.mesh',
  'base\\characters\\common\\hair\\h1_011_ma_hat__baseball_back_hh_04\\h1_011_ma_hat__baseball_back_hh_04.mesh',
  'base\\characters\\common\\hair\\h1_011_ma_hat__baseball_hh_01\\h1_011_ma_hat__baseball_hh_01.mesh',
  'base\\characters\\common\\hair\\h1_011_ma_hat__baseball_hh_03\\h1_011_ma_hat__baseball_hh_03.mesh',
  'base\\characters\\common\\hair\\h1_011_ma_hat__baseball_hh_04\\h1_011_ma_hat__baseball_hh_04.mesh',
  'base\\characters\\common\\hair\\h1_011_wa_hat__baseball_hh_02\\h1_011_ma_hat__baseball_hh_02.mesh',
  'base\\characters\\common\\hair\\h1_012_wa_hat__googles_hh_01\\h1_012_ma_hat__googles_hh_01.mesh',
  'base\\characters\\common\\hair\\h1_012_wa_hat__googles_hh_02\\h1_012_ma_hat__googles_hh_02.mesh',
  'base\\characters\\common\\hair\\h1_012_wa_hat__googles_hh_03\\h1_012_ma_hat__googles_hh_03.mesh',
  'base\\characters\\common\\hair\\h1_012_wa_hat__googles_hh_04\\h1_012_ma_hat__googles_hh_04.mesh',
  'base\\characters\\common\\hair\\h1_021_ma_hat__cowboy_sheriff_hh_01\\h1_021_ma_hat__cowboy_sheriff_hh_01.mesh',
  'base\\characters\\common\\hair\\h1_021_ma_hat__cowboy_sheriff_hh_03\\h1_021_ma_hat__cowboy_sheriff_hh_03.mesh',
  'base\\characters\\common\\hair\\h1_021_ma_hat__cowboy_sheriff_hh_04\\h1_021_ma_hat__cowboy_sheriff_hh_04.mesh',
  'base\\characters\\common\\hair\\h1_021_wa_hat__cowboy_sheriff_hh_02\\h1_021_ma_hat__cowboy_sheriff_hh_02.mesh',
  'base\\characters\\common\\hair\\h1_024_ma_hat__scavenger_hh_01\\h1_024_ma_hat__scavenger_hh_01.mesh',
  'base\\characters\\common\\hair\\h1_024_ma_hat__scavenger_hh_03\\h1_024_ma_hat__scavenger_hh_03.mesh',
  'base\\characters\\common\\hair\\h1_024_ma_hat__scavenger_hh_04\\h1_024_ma_hat__scavenger_hh_04.mesh',
  'base\\characters\\common\\hair\\h1_024_wa_hat__scavenger_hh_02\\h1_024_ma_hat__scavenger_hh_02.mesh',
  'base\\characters\\common\\hair\\h1_032_ma_hat__asian_hh_01\\h1_032_ma_hat__asian_hh_01.mesh',
  'base\\characters\\common\\hair\\h1_032_ma_hat__asian_hh_02\\h1_032_ma_hat__asian_hh_02.mesh',
  'base\\characters\\common\\hair\\h1_032_ma_hat__asian_hh_02\\h1_032_ma_hat__asian_hh_02_bun.mesh',
  'base\\characters\\common\\hair\\h1_032_ma_hat__asian_hh_03\\h1_032_ma_hat__asian_hh_03.mesh',
  'base\\characters\\common\\hair\\h1_032_ma_hat__asian_hh_04\\h1_032_ma_hat__asian_hh_04.mesh',
  'base\\characters\\common\\hair\\h1_036_wa_hat__elegant_hh_01\\h1_036_ma_hat__elegant_hh_01.mesh',
  'base\\characters\\common\\hair\\h1_036_wa_hat__elegant_hh_02\\h1_036_ma_hat__elegant_hh_02.mesh',
  'base\\characters\\common\\hair\\h1_036_wa_hat__elegant_hh_03\\h1_036_ma_hat__elegant_hh_03.mesh',
  'base\\characters\\common\\hair\\h1_036_wa_hat__elegant_hh_04\\h1_036_ma_hat__elegant_hh_04.mesh',
  'base\\characters\\common\\hair\\h1_045_ma_hat__farmer_hh_01\\h1_045_ma_hat__farmer_hh_01.mesh',
  'base\\characters\\common\\hair\\h1_045_ma_hat__farmer_hh_02\\h1_045_ma_hat__farmer_hh_02.mesh',
  'base\\characters\\common\\hair\\h1_045_ma_hat__farmer_hh_03\\h1_045_ma_hat__farmer_hh_03.mesh',
  'base\\characters\\common\\hair\\h1_045_ma_hat__farmer_hh_04\\h1_045_ma_hat__farmer_hh_04.mesh',
  'base\\characters\\common\\hair\\h1_051_ma_mask__headkerchief_hh_01\\h1_051_ma_mask__headkerchief_hh_01.mesh',
  'base\\characters\\common\\hair\\h1_051_ma_mask__headkerchief_hh_02\\h1_051_ma_mask__headkerchief_hh_02.mesh',
  'base\\characters\\common\\hair\\h1_051_ma_mask__headkerchief_hh_03\\h1_051_ma_mask__headkerchief_hh_03.mesh',
  'base\\characters\\common\\hair\\h1_051_ma_mask__headkerchief_hh_04\\h1_051_ma_mask__headkerchief_hh_04.mesh',
  'base\\characters\\common\\hair\\h1_060_ma_hat__bandana_kerry_01\\h1_060_ma_hat__bandana_kerry_01.mesh',
  'base\\characters\\common\\hair\\h1_060_ma_hat__bandana_kerry_hh_02\\h1_060_ma_hat__bandana_kerry_hh_02.mesh',
  'base\\characters\\common\\hair\\h1_060_ma_hat__bandana_kerry_hh_03\\h1_060_ma_hat__bandana_kerry_hh_03.mesh',
  'base\\characters\\common\\hair\\h1_060_ma_hat__bandana_kerry_hh_04\\h1_060_ma_hat__bandana_kerry_hh_04.mesh',
  'base\\characters\\common\\hair\\h1_063_ma_hat__military_hh_02\\h1_063_ma_hat__military_hh_02.mesh',
  'base\\characters\\common\\hair\\h1_063_ma_hat__military_hh_04\\h1_063_ma_hat__military_hh_04.mesh',
  'base\\characters\\common\\hair\\h1_063_wa_hat__military_hh_01\\h1_063_ma_hat__military_hh_01.mesh',
  'base\\characters\\common\\hair\\h1_063_wa_hat__military_hh_03\\h1_063_ma_hat__military_hh_03.mesh',
  'base\\characters\\common\\hair\\h1_065_ma_hat__guard_hh_01\\h1_065_ma_hat__guard_hh_01.mesh',
  'base\\characters\\common\\hair\\h1_065_ma_hat__guard_hh_02\\h1_065_ma_hat__guard_hh_02.mesh',
  'base\\characters\\common\\hair\\h1_065_ma_hat__guard_hh_03\\h1_065_ma_hat__guard_hh_03.mesh',
  'base\\characters\\common\\hair\\h1_065_ma_hat__guard_hh_04\\h1_065_ma_hat__guard_hh_04.mesh',
  'base\\characters\\common\\hair\\h1_082_ma_hat__bandage_hh_01\\h1_082_ma_hat__bandage_hh_01.mesh',
  'base\\characters\\common\\hair\\h1_082_ma_hat__bandage_hh_02\\h1_082_ma_hat__bandage_hh_02.mesh',
  'base\\characters\\common\\hair\\h1_082_ma_hat__bandage_hh_03\\h1_082_ma_hat__bandage_hh_03.mesh',
  'base\\characters\\common\\hair\\h1_082_ma_hat__bandage_hh_04\\h1_082_ma_hat__bandage_hh_04.mesh',
  'base\\characters\\common\\hair\\hh_083_wa__ponytail_01\\hh_083_ma__ponytail_base_01.mesh',
  'base\\characters\\common\\hair\\hh_083_wa__ponytail_01\\hh_083_ma__ponytail_braid_01.mesh',
  'base\\characters\\common\\hair\\hh_083_wa__ponytail_01\\hh_083_ma__ponytail_bulb_01.mesh',
  'base\\characters\\common\\hair\\hh_083_wa__ponytail_01\\hh_083_ma__ponytail_straight_01.mesh',
  'ep1\\characters\\common\\hair\\h1_002_mask__ikeuchi_hh_01\\h1_002_mask_ma__ikeuchi_hh_01.mesh',
  'ep1\\characters\\common\\hair\\h1_002_mask__ikeuchi_hh_04\\h1_002_mask_ma__ikeuchi_hh_04.mesh',
  'ep1\\characters\\common\\hair\\h1_002_mask__ikeuchi_hh_03\\h1_002_mask_ma__ikeuchi_hh_03.mesh',
  'ep1\\characters\\common\\hair\\h1_083_ma_hat__ushanka_hh_01\\h1_083_ma_hat__ushanka_hh_01.mesh',
  'ep1\\characters\\common\\hair\\h1_083_ma_hat__ushanka_hh_02\\h1_083_ma_hat__ushanka_hh_02.mesh',
  'ep1\\characters\\common\\hair\\h1_083_ma_hat__ushanka_hh_03\\h1_083_ma_hat__ushanka_hh_03.mesh',
  'ep1\\characters\\common\\hair\\h1_083_ma_hat__ushanka_hh_04\\h1_083_ma_hat__ushanka_hh_04.mesh',
  'ep1\\characters\\common\\hair\\h1_084_ma_hat__bucket_hat_hh_01\\h1_084_ma_hat__bucket_hat_hh_01.mesh',
  'ep1\\characters\\common\\hair\\h1_084_ma_hat__bucket_hat_hh_02\\h1_084_ma_hat__bucket_hat_hh_02.mesh',
  'ep1\\characters\\common\\hair\\h1_084_ma_hat__bucket_hat_hh_03\\h1_084_ma_hat__bucket_hat_hh_03.mesh',
  'ep1\\characters\\common\\hair\\h1_084_ma_hat__bucket_hat_hh_04\\h1_084_ma_hat__bucket_hat_hh_04.mesh',
]

const hairWaMainMeshes = [
  'base\\characters\\common\\hair\\hh_006_ma__demo\\hh_006_pwa__demo_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_006_ma__demo\\hh_006_wa__demo.mesh',
  'base\\characters\\common\\hair\\hh_008_ma__demo\\hh_008_pwa__sk8t_or_die_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_008_ma__demo\\hh_008_wa__sk8t_or_die.mesh',
  'base\\characters\\common\\hair\\hh_011_wa__demo\\hh_011_wa__demo.mesh',
  'base\\characters\\common\\hair\\hh_029_wa__misty\\hh_029_wa__misty_common.mesh',
  'base\\characters\\common\\hair\\hh_033_wa__player\\hh_033_pwa__player_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_033_wa__player\\hh_033_wa__player.mesh',
  'base\\characters\\common\\hair\\hh_034_wa__militech_agent\\hh_034_wa__militech_agent_common.mesh',
  'base\\characters\\common\\hair\\hh_035_ma__mohawk_tall\\hh_035_pwa__mohawk_tall_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_035_ma__mohawk_tall\\hh_035_wa__mohawk_tall.mesh',
  'base\\characters\\common\\hair\\hh_036_ma__high_tight\\hh_036_pwa__high_tight_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_036_ma__high_tight\\hh_036_wa__high_tight.mesh',
  'base\\characters\\common\\hair\\hh_039_wa__punk_shaved\\hh_039_pwa__punk_shaved_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_039_wa__punk_shaved\\hh_039_wa__punk_shaved.mesh',
  'base\\characters\\common\\hair\\hh_040_wa__pixie_bob\\hh_040_wa__pixie_bob.mesh',
  'base\\characters\\common\\hair\\hh_041_wa__valentino\\hh_041_pwa__valentino_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_041_wa__valentino\\hh_041_wa__valentino.mesh',
  'base\\characters\\common\\hair\\hh_042_wa__topknots\\hh_042_pwa__topknots_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_042_wa__topknots\\hh_042_wa__topknots.mesh',
  'base\\characters\\common\\hair\\hh_044_wa__classic\\hh_044_wa__classic.mesh',
  'base\\characters\\common\\hair\\hh_045_ma__short_spiked\\hh_045_pwa__short_spiked_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_045_ma__short_spiked\\hh_045_wa__short_spiked.mesh',
  'base\\characters\\common\\hair\\hh_049_ma__thiago\\hh_049_wa__thiago_common.mesh',
  'base\\characters\\common\\hair\\hh_051_wa__judy\\hh_051_pwa__judy_common_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_051_wa__judy\\hh_051_wa__judy_common.mesh',
  'base\\characters\\common\\hair\\hh_054_wa__rogue_young\\hh_054_pwa__rogue_young_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_054_wa__rogue_young\\hh_054_wa__rogue_young_common.mesh',
  'base\\characters\\common\\hair\\hh_059_wa__voodoo_02\\hh_059_pwa__voodoo_02_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_059_wa__voodoo_02\\hh_059_wa__voodoo_02.mesh',
  'base\\characters\\common\\hair\\hh_060_wa__voodoo_03\\hh_060_pwa__voodoo_03_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_060_wa__voodoo_03\\hh_060_wa__voodoo_03.mesh',
  'base\\characters\\common\\hair\\hh_063_wa_messy_bob\\hh_063_wa__messy_bob.mesh',
  'base\\characters\\common\\hair\\hh_064_wa__bob_fringe\\hh_064_wa__bob_fringe.mesh',
  'base\\characters\\common\\hair\\hh_068_wba__animals_03\\hh_068_wa__animals_03.mesh',
  'base\\characters\\common\\hair\\hh_077_wa__nomad_02\\hh_077_wa__nomad_02.mesh',
  'base\\characters\\common\\hair\\hh_078_wa__evelyn\\hh_078_wa__evelyn_common.mesh',
  'base\\characters\\common\\hair\\hh_079_wa__denny\\hh_079_pwa__denny.mesh',
  'base\\characters\\common\\hair\\hh_081_wa__buns_02\\hh_081_wa__buns_02.mesh',
  'base\\characters\\common\\hair\\hh_082_ma__afro_bun\\hh_082_pwa__afro_bun_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_082_ma__afro_bun\\hh_082_wa__afro_bun.mesh',
  'base\\characters\\common\\hair\\hh_082_ma__afro_bun\\hh_082_wa__afro_bun_bun.mesh',
  'base\\characters\\common\\hair\\hh_083_wa__ponytail_01\\hh_083_wa__ponytail_base_01.mesh',
  'base\\characters\\common\\hair\\hh_083_wa__ponytail_01\\hh_083_wa__ponytail_braid_01.mesh',
  'base\\characters\\common\\hair\\hh_083_wa__ponytail_01\\hh_083_wa__ponytail_bulb_01.mesh',
  'base\\characters\\common\\hair\\hh_083_wa__ponytail_01\\hh_083_wa__ponytail_straight_01.mesh',
  'base\\characters\\common\\hair\\hh_085_ma__takemura\\hh_085_wa__takemura_common.mesh',
  'base\\characters\\common\\hair\\hh_088_wa__corpo_bob\\hh_088_wa__corpo_bob.mesh',
  'base\\characters\\common\\hair\\hh_089_ma__thompson\\hh_089_pwa__thompson_common_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_089_ma__thompson\\hh_089_wa__thompson_common.mesh',
  'base\\characters\\common\\hair\\hh_090_wa__alt\\hh_090_wa__alt_player.mesh',
  'base\\characters\\common\\hair\\hh_091_wa__dakota\\hh_091_wa__dakota.mesh',
  'base\\characters\\common\\hair\\hh_091_wa__dakota\\hh_091_wa__dakota_braid.mesh',
  'base\\characters\\common\\hair\\hh_093_mba__sumo\\hh_093_wa__sumo.mesh',
  'base\\characters\\common\\hair\\hh_093_mba__sumo\\hh_093_wa__sumo_knot.mesh',
  'base\\characters\\common\\hair\\hh_103_ma__maelstrom_spikes\\hh_103_wa__common_spikes.mesh',
  'base\\characters\\common\\hair\\hh_106_wa__ponytails\\hh_106_wa__ponytails.mesh',
  'base\\characters\\common\\hair\\hh_115_wa__alanah\\hh_115_wa__alanah_common.mesh',
  'base\\characters\\common\\hair\\hh_118_wa__gillean\\hh_118_wa__gillean.mesh',
  'base\\characters\\common\\hair\\hh_121_wa__t_bug\\hh_121_wa__t_bug_common.mesh',
  'base\\characters\\common\\hair\\hh_140_ma__short_afro\\hh_140_pwa__short_afro_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_140_ma__short_afro\\hh_140_wa__short_afro.mesh',
  'base\\characters\\common\\hair\\hh_141_ma__afro\\hh_141_wa__afro.mesh',
  'base\\characters\\common\\hair\\hh_142_ma__afrohawk\\hh_142_pwa__afrohawk.mesh',
  'base\\characters\\common\\hair\\hh_142_ma__afrohawk\\hh_142_pwa__afrohawk_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_143_ma__flat_top\\hh_143_pwa__flat_top_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_143_ma__flat_top\\hh_143_wa__flat_top.mesh',
  'base\\characters\\common\\hair\\hh_144_wa__afro_braid_bun\\hh_144_pwa__afro_braid_bun_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_144_wa__afro_braid_bun\\hh_144_wa__afro_braid_bun.mesh',
  'base\\characters\\common\\hair\\hh_146_ma__dread_undercut\\hh_146_pwa__dread_undercut_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_146_ma__dread_undercut\\hh_146_wa__dread_undercut.mesh',
  'base\\characters\\common\\hair\\hh_151_wa__judy_variation02\\hh_151_pwa__judy_variation02_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_151_wa__judy_variation02\\hh_151_wa__judy_variation02_common.mesh',
  'base\\characters\\common\\hair\\hh_999_ma__buzz_cap\\hh_999_pwa__buzz_cap_cyberware_01.mesh',
  'base\\characters\\common\\hair\\hh_999_ma__buzz_cap\\hh_999_wa__buzz_cap.mesh',
]

const hairWaHatMeshes = [
  'base\\characters\\common\\hair\\h1_003_wa_hat__beanie_hh_01\\h1_003_wa_hat__beanie_hh_01.mesh',
  'base\\characters\\common\\hair\\h1_003_wa_hat__beanie_hh_02\\h1_003_wa_hat__beanie_hh_02.mesh',
  'base\\characters\\common\\hair\\h1_003_wa_hat__beanie_hh_03\\h1_003_wa_hat__beanie_hh_03.mesh',
  'base\\characters\\common\\hair\\h1_003_wa_hat__beanie_hh_04\\h1_003_wa_hat__beanie_hh_04.mesh',
  'base\\characters\\common\\hair\\h1_011_ma_hat__baseball_back_hh_01\\h1_011_wa_hat__baseball_back_hh_01.mesh',
  'base\\characters\\common\\hair\\h1_011_ma_hat__baseball_back_hh_02\\h1_011_wa_hat__baseball_back_hh_02.mesh',
  'base\\characters\\common\\hair\\h1_011_ma_hat__baseball_back_hh_03\\h1_011_wa_hat__baseball_back_hh_03.mesh',
  'base\\characters\\common\\hair\\h1_011_ma_hat__baseball_back_hh_04\\h1_011_wa_hat__baseball_back_hh_04.mesh',
  'base\\characters\\common\\hair\\h1_011_ma_hat__baseball_hh_01\\h1_011_wa_hat__baseball_hh_01.mesh',
  'base\\characters\\common\\hair\\h1_011_ma_hat__baseball_hh_03\\h1_011_wa_hat__baseball_hh_03.mesh',
  'base\\characters\\common\\hair\\h1_011_ma_hat__baseball_hh_04\\h1_011_wa_hat__baseball_hh_04.mesh',
  'base\\characters\\common\\hair\\h1_011_wa_hat__baseball_hh_02\\h1_011_wa_hat__baseball_hh_02.mesh',
  'base\\characters\\common\\hair\\h1_012_wa_hat__googles_hh_01\\h1_012_wa_hat__googles_hh_01.mesh',
  'base\\characters\\common\\hair\\h1_012_wa_hat__googles_hh_02\\h1_012_wa_hat__googles_hh_02.mesh',
  'base\\characters\\common\\hair\\h1_012_wa_hat__googles_hh_03\\h1_012_wa_hat__googles_hh_03.mesh',
  'base\\characters\\common\\hair\\h1_012_wa_hat__googles_hh_04\\h1_012_wa_hat__googles_hh_04.mesh',
  'base\\characters\\common\\hair\\h1_021_ma_hat__cowboy_sheriff_hh_01\\h1_021_wa_hat__cowboy_sheriff_hh_01.mesh',
  'base\\characters\\common\\hair\\h1_021_ma_hat__cowboy_sheriff_hh_03\\h1_021_wa_hat__cowboy_sheriff_hh_03.mesh',
  'base\\characters\\common\\hair\\h1_021_ma_hat__cowboy_sheriff_hh_04\\h1_021_wa_hat__cowboy_sheriff_hh_04.mesh',
  'base\\characters\\common\\hair\\h1_021_wa_hat__cowboy_sheriff_hh_02\\h1_021_wa_hat__cowboy_sheriff_hh_02.mesh',
  'base\\characters\\common\\hair\\h1_024_ma_hat__scavenger_hh_01\\h1_024_wa_hat__scavenger_hh_01.mesh',
  'base\\characters\\common\\hair\\h1_024_ma_hat__scavenger_hh_03\\h1_024_wa_hat__scavenger_hh_03.mesh',
  'base\\characters\\common\\hair\\h1_024_ma_hat__scavenger_hh_04\\h1_024_wa_hat__scavenger_hh_04.mesh',
  'base\\characters\\common\\hair\\h1_024_wa_hat__scavenger_hh_02\\h1_024_wa_hat__scavenger_hh_02.mesh',
  'base\\characters\\common\\hair\\h1_032_ma_hat__asian_hh_01\\h1_032_wa_hat__asian_hh_01.mesh',
  'base\\characters\\common\\hair\\h1_032_ma_hat__asian_hh_02\\h1_032_wa_hat__asian_hh_02.mesh',
  'base\\characters\\common\\hair\\h1_032_ma_hat__asian_hh_02\\h1_032_wa_hat__asian_hh_02_bun.mesh',
  'base\\characters\\common\\hair\\h1_032_ma_hat__asian_hh_03\\h1_032_wa_hat__asian_hh_03.mesh',
  'base\\characters\\common\\hair\\h1_032_ma_hat__asian_hh_04\\h1_032_wa_hat__asian_hh_04.mesh',
  'base\\characters\\common\\hair\\h1_036_wa_hat__elegant_hh_01\\h1_036_wa_hat__elegant_hh_01.mesh',
  'base\\characters\\common\\hair\\h1_036_wa_hat__elegant_hh_02\\h1_036_wa_hat__elegant_hh_02.mesh',
  'base\\characters\\common\\hair\\h1_036_wa_hat__elegant_hh_03\\h1_036_wa_hat__elegant_hh_03.mesh',
  'base\\characters\\common\\hair\\h1_036_wa_hat__elegant_hh_04\\h1_036_wa_hat__elegant_hh_04.mesh',
  'base\\characters\\common\\hair\\h1_045_ma_hat__farmer_hh_01\\h1_045_wa_hat__farmer_hh_01.mesh',
  'base\\characters\\common\\hair\\h1_045_ma_hat__farmer_hh_02\\h1_045_wa_hat__farmer_hh_02.mesh',
  'base\\characters\\common\\hair\\h1_045_ma_hat__farmer_hh_03\\h1_045_wa_hat__farmer_hh_03.mesh',
  'base\\characters\\common\\hair\\h1_045_ma_hat__farmer_hh_04\\h1_045_wa_hat__farmer_hh_04.mesh',
  'base\\characters\\common\\hair\\h1_051_ma_mask__headkerchief_hh_01\\h1_051_wa_mask__headkerchief_hh_01.mesh',
  'base\\characters\\common\\hair\\h1_051_ma_mask__headkerchief_hh_02\\h1_051_wa_mask__headkerchief_hh_02.mesh',
  'base\\characters\\common\\hair\\h1_051_ma_mask__headkerchief_hh_03\\h1_051_wa_mask__headkerchief_hh_03.mesh',
  'base\\characters\\common\\hair\\h1_051_ma_mask__headkerchief_hh_04\\h1_051_wa_mask__headkerchief_hh_04.mesh',
  'base\\characters\\common\\hair\\h1_060_ma_hat__bandana_kerry_01\\h1_060_wa_hat__bandana_kerry_01.mesh',
  'base\\characters\\common\\hair\\h1_060_ma_hat__bandana_kerry_hh_02\\h1_060_wa_hat__bandana_kerry_hh_02.mesh',
  'base\\characters\\common\\hair\\h1_060_ma_hat__bandana_kerry_hh_03\\h1_060_wa_hat__bandana_kerry_hh_03.mesh',
  'base\\characters\\common\\hair\\h1_060_ma_hat__bandana_kerry_hh_04\\h1_060_wa_hat__bandana_kerry_hh_04.mesh',
  'base\\characters\\common\\hair\\h1_063_ma_hat__military_hh_02\\h1_063_wa_hat__military_hh_02.mesh',
  'base\\characters\\common\\hair\\h1_063_ma_hat__military_hh_04\\h1_063_wa_hat__military_hh_04.mesh',
  'base\\characters\\common\\hair\\h1_063_wa_hat__military_hh_01\\h1_063_wa_hat__military_hh_01.mesh',
  'base\\characters\\common\\hair\\h1_063_wa_hat__military_hh_03\\h1_063_wa_hat__military_hh_03.mesh',
  'base\\characters\\common\\hair\\h1_065_ma_hat__guard_hh_01\\h1_065_wa_hat__guard_hh_01.mesh',
  'base\\characters\\common\\hair\\h1_065_ma_hat__guard_hh_02\\h1_065_wa_hat__guard_hh_02.mesh',
  'base\\characters\\common\\hair\\h1_065_ma_hat__guard_hh_03\\h1_065_wa_hat__guard_hh_03.mesh',
  'base\\characters\\common\\hair\\h1_065_ma_hat__guard_hh_04\\h1_065_wa_hat__guard_hh_04.mesh',
  'base\\characters\\common\\hair\\h1_082_ma_hat__bandage_hh_01\\h1_082_wa_hat__bandage_hh_01.mesh',
  'base\\characters\\common\\hair\\h1_082_ma_hat__bandage_hh_02\\h1_082_wa_hat__bandage_hh_02.mesh',
  'base\\characters\\common\\hair\\h1_082_ma_hat__bandage_hh_03\\h1_082_wa_hat__bandage_hh_03.mesh',
  'base\\characters\\common\\hair\\h1_082_ma_hat__bandage_hh_04\\h1_082_wa_hat__bandage_hh_04.mesh',
  'ep1\\characters\\common\\hair\\h1_002_mask__ikeuchi_hh_01\\h1_002_mask_wa__ikeuchi_hh_01.mesh',
  'ep1\\characters\\common\\hair\\h1_002_mask__ikeuchi_hh_03\\h1_002_mask_wa__ikeuchi_hh_03.mesh',
  'ep1\\characters\\common\\hair\\h1_002_mask__ikeuchi_hh_04\\h1_002_mask_wa__ikeuchi_hh_04.mesh',
  'ep1\\characters\\common\\hair\\h1_002_mask_wa__ikeuchi_hh_02\\h1_002_mask_wa__ikeuchi_hh_02.mesh',
  'ep1\\characters\\common\\hair\\h1_083_ma_hat__ushanka_hh_01\\h1_083_wa_hat__ushanka_hh_01.mesh',
  'ep1\\characters\\common\\hair\\h1_083_ma_hat__ushanka_hh_02\\h1_083_wa_hat__ushanka_hh_02.mesh',
  'ep1\\characters\\common\\hair\\h1_083_ma_hat__ushanka_hh_03\\h1_083_wa_hat__ushanka_hh_03.mesh',
  'ep1\\characters\\common\\hair\\h1_083_ma_hat__ushanka_hh_04\\h1_083_wa_hat__ushanka_hh_04.mesh',
  'ep1\\characters\\common\\hair\\h1_084_ma_hat__bucket_hat_hh_01\\h1_084_wa_hat__bucket_hat_hh_01.mesh',
  'ep1\\characters\\common\\hair\\h1_084_ma_hat__bucket_hat_hh_02\\h1_084_wa_hat__bucket_hat_hh_02.mesh',
  'ep1\\characters\\common\\hair\\h1_084_ma_hat__bucket_hat_hh_03\\h1_084_wa_hat__bucket_hat_hh_03.mesh',
  'ep1\\characters\\common\\hair\\h1_084_ma_hat__bucket_hat_hh_04\\h1_084_wa_hat__bucket_hat_hh_04.mesh',
]

const hairMeshes = [
  ...hairMaMainMeshes,
  ...hairMaHatMeshes,
  ...hairWaMainMeshes,
  ...hairWaHatMeshes,
]

const usedMasterMaterials = new Set()
const extensionData = {}

if (logInfo || logWarnings) {
  Logger.Info('Converting hair meshes...')
}

for (let meshPath of hairMeshes) {
  let meshFile = wkit.GetFileFromBase(meshPath)

  if (!meshFile) {
    Logger.Error(`Mesh cannot be opened: ${meshPath}`)
    continue
  }

  let meshJson = wkit.GameFileToJson(meshFile)

  if (!meshJson) {
    Logger.Error(`Mesh cannot be parsed: ${meshPath}`)
    continue
  }

  let meshRaw = TypeHelper.JsonParse(meshJson)
  let meshInstance = meshRaw['Data']['RootChunk']

  let contextParams = {}
  let materialMappings = {}

  if (logInfo || logWarnings) {
    Logger.Info(`${meshPath}`)

    if (logInfo) {
      Logger.Info('  Materials:')
    }
  }

  for (let materialEntry of meshInstance['materialEntries']) {
    const materialOldName = materialEntry['name'].toString()

    if (materialOldName === 'hologram') {
      if (materialEntry['isLocalInstance']) {
        materialEntry['index']++ // compensate for @context material entry
      }
      continue
    }

    let materialData = {}
    let materialInstance
    if (materialEntry['isLocalInstance']) {
      materialInstance = meshInstance['localMaterialBuffer']['materials'][materialEntry['index']]
      materialEntry['index']++ // compensate for @context material entry
    } else {
      let materialPath = meshInstance['externalMaterials'][materialEntry['index']]['DepotPath'].toString()
      materialInstance = loadMaterialInstace(materialPath)
    }

    collectMaterialData(materialInstance, materialData, contextParams)

    if (!materialData['template']) {
      continue
    }

    materialEntry['name'] = new CName(materialData['name'] + materialData['template'])
    materialMappings[materialOldName] = materialEntry['name']

    if (logInfo) {
      Logger.Info(`  - ${materialEntry['name']}`)
    }

    if (materialData['master']) {
      usedMasterMaterials.add(materialData['master'])
    }
  }

  if (logInfo) {
    Logger.Info('  Context:')
    for (let paramName in contextParams) {
      Logger.Info(`  ${paramName} = ${contextParams[paramName]}`)
    }
  }

  if (generateFixedResources) {
    const contextInstance = {
      '$type': 'CMaterialInstance',
      'values': []
    }

    for (let paramName in contextParams) {
      contextInstance.values.push({
        '$type': 'CName',
        [paramName]: {
          '$type': 'CName',
          '$storage': 'string',
          '$value': contextParams[paramName],
        }
      })
    }

    if (!meshInstance['localMaterialBuffer']['materials']) {
      meshInstance['localMaterialBuffer']['materials'] = []
    }

    meshInstance['localMaterialBuffer']['materials'].unshift(contextInstance)
    meshInstance['materialEntries'].unshift({
      '$type': 'CMeshMaterialEntry',
      'index': 0,
      'isLocalInstance': 1,
      'name': {
        '$type': 'CName',
        '$storage': 'string',
        '$value': '@context'
      }
    })

    const unmappedMaterials = new Set()
    for (let i = 0, n = meshInstance['appearances'].length; i < n; ++i) {
      const chunkMaterials = meshInstance['appearances'][i]['Data']['chunkMaterials']
      for (let j = 0, m = chunkMaterials.length; j < m; ++j) {
        if (materialMappings.hasOwnProperty(chunkMaterials[j])) {
          chunkMaterials[j] = materialMappings[chunkMaterials[j]]
        } else {
          if (!unmappedMaterials.has(chunkMaterials[j].toString())) {
            unmappedMaterials.add(chunkMaterials[j].toString())
            if (logInfo || logWarnings) {
              (i === 0 ? Logger.Error : Logger.Warning)(`No mapping for material: ${chunkMaterials[j]}`)
            }
          }
        }
      }
    }

    meshJson = TypeHelper.JsonStringify(meshRaw)
    wkit.SaveToProject(meshPath, wkit.JsonToCR2W(meshJson))
  }

  if (generateExtensionJson) {
    const names = {}
    for (const materialName in materialMappings) {
      if (materialMappings.hasOwnProperty(materialName)) {
        names[materialName] = materialMappings[materialName].toString()
      }
    }
    extensionData[meshPath] = {
      names: names,
      context: contextParams,
    }
  }
}

if (logInfo || logWarnings) {
  Logger.Info('Used master materials:')
  for (let materialPath of usedMasterMaterials) {
    Logger.Info(`- ${materialPath}`)
  }

  Logger.Success('Converion finished.')
}

if (generateExtensionJson) {
  wkit.SaveToRaw('PlaeyrHairFix.json', JSON.stringify({resource: {fix: extensionData}}))
}

function collectMaterialData(materialInstance, materialData, contextParams) {
  let basePath = materialInstance['baseMaterial']['DepotPath'].toString()
  if (!basePath.match(/\.(?:re)?mt$/)) {
    let baseInstance = loadMaterialInstace(basePath)

    collectMaterialData(baseInstance, materialData, contextParams)

    let masterPathMatch = basePath.match(/^base\\characters\\common\\hair\\textures\\hair_profiles\\_master__([a-z]+)(?:_bright)?_?\.mi$/)
    if (masterPathMatch) {
      materialData['template'] = '@' + masterPathMatch[1]
      materialData['master'] = basePath
    }
  }

  for (let materialParam of materialInstance['values']) {
    if (materialParam.hasOwnProperty('HairProfile')) {
      let profilePath = materialParam['HairProfile']['DepotPath'].toString()
      let profileMatch = profilePath.match(/\\([^\\]+)\.hp$/)
      if (profileMatch) {
        materialData['name'] = profileMatch[1]
      }
      continue
    }

    if (materialParam.hasOwnProperty('DiffuseTexture')) {
      let texturePath = materialParam['DiffuseTexture']['DepotPath'].toString()
      if (texturePath && !texturePath.match(/^engine\\textures\\editor\\/)) {
        contextParams['CapDiffuseTexture'] = texturePath
        materialData['template'] = '@cap'
      }
      continue
    }

    if (materialParam.hasOwnProperty('MaskTexture')) {
      let texturePath = materialParam['MaskTexture']['DepotPath'].toString()
      if (texturePath && !texturePath.match(/^engine\\textures\\editor\\/)) {
        contextParams['CapMaskTexture'] = texturePath
        materialData['template'] = '@cap'
      }
      continue
    }

    if (materialParam.hasOwnProperty('SecondaryMask')) {
      let texturePath = materialParam['SecondaryMask']['DepotPath'].toString()
      if (texturePath && !texturePath.match(/^engine\\textures\\editor\\/)) {
        contextParams['CapSecondaryMaskTexture'] = texturePath
        materialData['template'] = '@cap01'
      }
      continue
    }

    if (materialParam.hasOwnProperty('GradientMap')) {
      let gradientPath = materialParam['GradientMap']['DepotPath'].toString()
      let gradientMatch = gradientPath.match(/^base\\characters\\common\\hair\\textures\\cap_gradiants\\hh_cap_grad__([^.]+)\.xbm$/)
      if (gradientMatch) {
        materialData['name'] = gradientMatch[1]
      }
    }
  }
}

function loadMaterialInstace(materialPath) {
  let materialFile = wkit.GetFileFromBase(materialPath)
  let materialRaw = TypeHelper.JsonParse(wkit.GameFileToJson(materialFile))
  return materialRaw['Data']['RootChunk']
}
