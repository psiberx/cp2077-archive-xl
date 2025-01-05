#include "Tags.hpp"

App::OverrideTagManager::OverrideTagManager() noexcept
    : m_definitions({
          {
              "hide_Head",
              {
                  {"h0_", Hide()},
                  {"he_", Hide()},
                  {"heb_", Hide()},
                  {"ht_", Hide()},
                  {"hx_", Hide()},
                  {"i1_", Hide()}, // accessories
                  {"beard", Hide()},
                  {"beard_", Hide()},
                  {"MorphTargetSkinnedMesh3637", Hide()},
                  {"MorphTargetSkinnedMesh6675", Hide()},
                  {"MorphTargetSkinnedMesh7243", Hide()},
                  {"MorphTargetSkinnedMesh7561", Hide()},
              }
          },
          {
              "hide_Arms",
              {
                  {"a0_", Hide()},
                  {"left_arm", Hide()},
                  {"right_arm", Hide()},
              }
          },
          {
              "hide_Torso",
              {
                  {"n0_", Hide()},
                  {"tx_", Hide()},
                  {"t0_000_pma_base__full", Hide({0, 1, 2, 3})},
                  {"t0_000_pma_base__full_seamfix", Hide()},
                  {"t0_000_pwa_base__full", Hide({0, 1, 2, 3})},
                  {"t0_000_pwa_base__full_seamfix", Hide()},
                  {"t0_000_pwa_fpp__torso", Hide({0, 1, 2, 3})},
                  {"MorphTargetSkinnedMesh0531", Hide()}, // nipples
              }
          },
          {
              "hide_LowerAbdomen",
              {
                  {"t0_000_pma_base__full", Hide({3})},
                  {"t0_000_pwa_base__full", Hide({3})},
                  {"t0_000_pwa_fpp__torso", Hide({3})},
              }
          },
          {
              "hide_UpperAbdomen",
              {
                  {"t0_000_pma_base__full", Hide({2})},
                  {"t0_000_pwa_base__full", Hide({2})},
                  {"t0_000_pwa_fpp__torso", Hide({2})},
              }
          },
          {
              "hide_CollarBone",
              {
                  {"t0_000_pma_base__full", Hide({1})},
                  {"t0_000_pwa_base__full", Hide({1})},
                  {"t0_000_pwa_fpp__torso", Hide({1})},
              }
          },
          {
              "hide_Chest",
              {
                  {"t0_000_pma_base__full", Hide({0})},
                  {"t0_000_pwa_base__full", Hide({0})},
                  {"t0_000_pwa_fpp__torso", Hide({0})},
                  {"MorphTargetSkinnedMesh0531", Hide()}, // nipples
              }
          },
          {
              "hide_Legs",
              {
                  {"l0_", Hide()},
                  {"s0_", Hide()},
                  {"t0_000_pma_base__full", Hide({4, 5, 6, 7})},
                  {"t0_000_pwa_base__full", Hide({4, 5, 6, 7})},
                  {"t0_000_pwa_fpp__torso", Hide({4, 5, 6, 7})},
              }
          },
          {
              "hide_Thighs",
              {
                  {"t0_000_pma_base__full", Hide({4})},
                  {"t0_000_pwa_base__full", Hide({4})},
                  {"t0_000_pwa_fpp__torso", Hide({4})},
              }
          },
          {
              "hide_Calves",
              {
                  {"t0_000_pma_base__full", Hide({5})},
                  {"l0_000_pma_base__high_heels", Hide({0})},
                  {"l0_000_pma_base__flat_shoes", Hide({0})},
                  {"t0_000_pwa_base__full", Hide({5})},
                  {"t0_000_pwa_fpp__torso", Hide({5})},
                  {"l0_000_pwa_base__cs_flat", Hide({0})},
                  {"l0_000_pwa_base__high_heels", Hide({0})},
                  {"l0_000_pwa_base__flat_shoes", Hide({0})},
              }
          },
          {
              "hide_Ankles",
              {
                  {"s0_", Hide()},
                  {"t0_000_pma_base__full", Hide({6})},
                  {"l0_000_pma_base__high_heels", Hide({1})},
                  {"l0_000_pma_base__flat_shoes", Hide({1})},
                  {"t0_000_pwa_base__full", Hide({6})},
                  {"t0_000_pwa_fpp__torso", Hide({6})},
                  {"l0_000_pwa_base__cs_flat", Hide({1})},
                  {"l0_000_pwa_base__high_heels", Hide({1})},
                  {"l0_000_pwa_base__flat_shoes", Hide({1})},
              }
          },
          {
              "hide_Feet",
              {
                  {"t0_000_pma_base__full", Hide({7})},
                  {"l0_000_pma_base__high_heels", Hide({2})},
                  {"l0_000_pma_base__flat_shoes", Hide({2})},
                  {"t0_000_pwa_base__full", Hide({7})},
                  {"t0_000_pwa_fpp__torso", Hide({7})},
                  {"l0_000_pwa_base__cs_flat", Hide({2})},
                  {"l0_000_pwa_base__high_heels", Hide({2})},
                  {"l0_000_pwa_base__flat_shoes", Hide({2})},
              }
          },
          {
              "HighHeels",
              {
                  {"t0_000_pma_base__full", Hide({5, 6, 7})},
                  {"l0_000_pma_base__high_heels", Show({0, 1, 2})},
                  {"t0_000_pwa_base__full", Hide({5, 6, 7})},
                  {"t0_000_pwa_fpp__torso", Hide({5, 6, 7})},
                  {"l0_000_pwa_base__high_heels", Show({0, 1, 2})},
              }
          },
          {
              "FlatShoes",
              {
                  {"t0_000_pma_base__full", Hide({5, 6, 7})},
                  {"l0_000_pma_base__flat_shoes", Show({0, 1, 2})},
                  {"t0_000_pwa_base__full", Hide({5, 6, 7})},
                  {"t0_000_pwa_fpp__torso", Hide({5, 6, 7})},
                  {"l0_000_pwa_base__flat_shoes", Show({0, 1, 2})},
              }
          },
      })
{
}

App::OverrideTagDefinition& App::OverrideTagManager::GetOverrides(Red::CName aTag)
{
    static OverrideTagDefinition s_emptyDefinition;

    const auto it = m_definitions.find(aTag);

    if (it == m_definitions.end())
        return s_emptyDefinition;

    return it.value();
}

void App::OverrideTagManager::DefineTag(Red::CName aTag, App::OverrideTagDefinition aDefinition)
{
    m_definitions[aTag] = std::move(aDefinition);
}
