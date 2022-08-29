#include "Tags.hpp"

App::OverrideTagManager::OverrideTagManager() noexcept
    : m_definitions({
          {
              "hide_Head",
              {
                  {"h0_", 0},
                  {"he_", 0},
                  {"heb_", 0},
                  {"ht_", 0},
                  {"hx_", 0},
                  {"i1_", 0},
                  {"MorphTargetSkinnedMesh3637", 0},
                  {"MorphTargetSkinnedMesh6675", 0},
                  {"MorphTargetSkinnedMesh7243", 0},
                  {"MorphTargetSkinnedMesh0531", 0},
              }
          },
          {
              "hide_Torso",
              {
                  {"n0_", 0},
                  {"tx_", 0},
                  {"body", ~0b1111},
                  {"t0_000_pma_base__full", ~0b1111},
                  {"t0_000_pwa_base__full", ~0b1111},
                  {"t0_000_pwa_base__full_seamfix", 0},
                  {"t0_000_pwa_fpp__torso", ~0b1111},
              }
          },
          {
              "hide_Arms",
              {
                  {"a0_", ~0b1111},
                  {"left_arm", 0},
                  {"right_arm", 0},
              }
          },
          {
              "hide_Legs",
              {
                  {"l0_", 0},
                  {"s0_", 0},
                  {"t0_000_pma_base__full", ~0b11110000},
                  {"t0_000_pwa_base__full", ~0b11110000},
                  {"t0_000_pwa_fpp__torso", ~0b11110000},
              }
          },
      })
{
}

App::OverrideTagDefinition& App::OverrideTagManager::GetOverrides(RED4ext::CName aTag)
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
