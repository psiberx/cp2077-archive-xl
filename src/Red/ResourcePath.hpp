#pragma once

namespace Red
{
struct StringView
{
    const char* data;
    uint32_t size;
};
}

namespace Raw::ResourcePath
{
constexpr auto Create = Core::RawFunc<
    /* addr = */ Red::AddressLib::ResourcePath_Create,
    /* type = */ Red::ResourcePath* (*)(Red::ResourcePath* aOut, const Red::StringView* aPath)>();
}
