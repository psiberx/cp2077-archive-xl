#pragma once

namespace Red
{
struct EntitySpawnerRequest
{
    uint8_t unk00[0xC0];        // 00
    CName appearanceName;       // C0
    uint8_t unkC8[0xE0 - 0xC8]; // C8
    TweakDBID recordID;         // E0
};
RED4EXT_ASSERT_OFFSET(EntitySpawnerRequest, appearanceName, 0xC0);
RED4EXT_ASSERT_OFFSET(EntitySpawnerRequest, recordID, 0xE0);
}

namespace Raw::EntitySpawner
{
constexpr auto SpawnFromTemplate = Core::RawFunc<
    /* addr = */ Red::AddressLib::EntitySpawner_SpawnFromTemplate,
    /* type = */ void* (*)(void* aSpawner, void* aOut, Red::EntitySpawnerRequest* aRequest,
                           Red::ResourcePath aTemplate)>();
}
