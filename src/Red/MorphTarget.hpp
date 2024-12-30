#pragma once

namespace Raw::MorphTargetMesh
{
constexpr auto PostLoad = Core::RawFunc<
    /* addr = */ Red::AddressLib::MorphTargetMesh_PostLoad,
    /* type = */ void (*)(Red::MorphTargetMesh* aResource, Red::PostLoadParams* a2)>();
}

namespace Raw::MorphTargetManager
{
constexpr auto ApplyMorphTarget = Core::RawFunc<
    /* addr = */ Red::AddressLib::MorphTargetManager_ApplyMorphTarget,
    /* type = */ void (*)(Red::IComponent* aManager, Red::CName aTarget, Red::CName aRegion, float aValue, bool a5)>();
}
