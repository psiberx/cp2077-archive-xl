#pragma once

namespace Red
{
using EntityTemplate = ent::EntityTemplate;
using TemplateAppearance = ent::TemplateAppearance;
}

namespace Raw::EntityTemplate
{
using BufferMask = Core::OffsetPtr<0x1D8, int16_t>;

constexpr auto OnLoad = Core::RawFunc<
    /* addr = */ Red::AddressLib::EntityTemplate_OnLoad,
    /* type = */ void (*)(Red::EntityTemplate* aResource, void* a2)>();

constexpr auto FindAppearance = Core::RawFunc<
    /* addr = */ Red::AddressLib::EntityTemplate_FindAppearance,
    /* type = */ Red::TemplateAppearance* (*)(Red::EntityTemplate* aResource, Red::CName aName)>();

constexpr auto ExtractBufferObjects = Core::RawFunc<
    /* addr = */ Red::AddressLib::EntityTemplate_ExtractBufferObjects,
    /* type = */ void* (*)(Red::DynArray<Red::Handle<Red::ISerializable>>& aOut,
                           const Red::SharedPtr<Red::ResourceToken<Red::EntityTemplate>>& aToken)>();
}
