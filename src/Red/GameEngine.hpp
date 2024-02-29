#pragma once

namespace Raw::CBaseEngine
{
constexpr auto LoadGatheredResources = Core::RawFunc<
    /* addr = */ Red::AddressLib::CBaseEngine_LoadGatheredResources,
    /* type = */ bool (*)(Red::CBaseEngine& aEngine)>();
}
