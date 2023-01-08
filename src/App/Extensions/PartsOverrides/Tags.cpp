#include "Tags.hpp"

App::OverrideTagManager::OverrideTagManager() noexcept
    : m_definitions({
          {
              "hide_Head",
              {
                  {"h0_", {}},
                  {"he_", {}},
                  {"heb_", {}},
                  {"ht_", {}},
                  {"hx_", {}},
                  {"i1_", {}}, // accessories
                  {"MorphTargetSkinnedMesh3637", {}},
                  {"MorphTargetSkinnedMesh6675", {}},
                  {"MorphTargetSkinnedMesh7243", {}},
                  {"MorphTargetSkinnedMesh7561", {}},
              }
          },
          {
              "hide_Arms",
              {
                  {"a0_", {}},
                  {"left_arm", {}},
                  {"right_arm", {}},
              }
          },
          {
              "hide_Torso",
              {
                  {"n0_", {}},
                  {"tx_", {}},
                  {"t0_000_pma_base__full", {0, 1, 2, 3}},
                  {"t0_000_pma_base__full_seamfix", {}},
                  {"t0_000_pwa_base__full", {0, 1, 2, 3}},
                  {"t0_000_pwa_base__full_seamfix", {}},
                  {"t0_000_pwa_fpp__torso", {0, 1, 2, 3}},
                  {"MorphTargetSkinnedMesh0531", {}}, // nipples
              }
          },
          {
              "hide_Legs",
              {
                  {"l0_", {}},
                  {"s0_", {}},
                  {"t0_000_pma_base__full", {4, 5, 6, 7}},
                  {"t0_000_pwa_base__full", {4, 5, 6, 7}},
                  {"t0_000_pwa_fpp__torso", {4, 5, 6, 7}},
              }
          },
          {
              "hide_Thighs",
              {
                  {"t0_000_pma_base__full", {4}},
                  {"t0_000_pwa_base__full", {4}},
                  {"t0_000_pwa_fpp__torso", {4}},
              }
          },
          {
              "hide_Calves",
              {
                  {"t0_000_pma_base__full", {5}},
                  {"t0_000_pwa_base__full", {5}},
                  {"t0_000_pwa_fpp__torso", {5}},
                  {"l0_000_pwa_base__cs_flat", {0}},
                  {"l0_000_pwa_base__cs_heel", {0}},
              }
          },
          {
              "hide_Ankles",
              {
                  {"s0_", {}},
                  {"t0_000_pma_base__full", {6}},
                  {"t0_000_pwa_base__full", {6}},
                  {"t0_000_pwa_fpp__torso", {6}},
                  {"l0_000_pwa_base__cs_flat", {1}},
                  {"l0_000_pwa_base__cs_heel", {1}},
              }
          },
          {
              "hide_Feet",
              {
                  {"t0_000_pma_base__full", {7}},
                  {"t0_000_pwa_base__full", {7}},
                  {"t0_000_pwa_fpp__torso", {7}},
                  {"l0_000_pwa_base__cs_flat", {2}},
                  {"l0_000_pwa_base__cs_heel", {2}},
              }
          },
          {
              "HighHeels",
              {
                  {"t0_000_pwa_base__full", {5, 6, 7}},
                  {"t0_000_pwa_fpp__torso", {5, 6, 7}},
                  {"l0_000_pwa_base__cs_heel", {true, {0, 1, 2}}},
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

Core::SharedPtr<App::OverrideTagManager>& App::OverrideTagManager::Get()
{
    static auto s_instance = Core::MakeShared<OverrideTagManager>();
    return s_instance;
}
