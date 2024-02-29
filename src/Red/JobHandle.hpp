#pragma once

namespace Raw::JobHandle
{
constexpr auto Wait = Core::RawFunc<
    /* addr = */ Red::AddressLib::JobHandle_Wait,
    /* type = */ bool (*)(Red::JobHandle& aJob)>();
}
