#pragma once

#include "Red/Package.hpp"

namespace Red
{
using EntityTemplate = ent::EntityTemplate;
using TemplateAppearance = ent::TemplateAppearance;
}

namespace Raw::EntityTemplate
{
using PackageData = Core::OffsetPtr<0x1D8, Red::ObjectPackageHeader>;

constexpr auto OnLoad = Core::RawFunc<
    /* addr = */ Red::AddressLib::EntityTemplate_OnLoad,
    /* type = */ void (*)(Red::EntityTemplate* aResource, void* a2)>();

constexpr auto FindAppearance = Core::RawFunc<
    /* addr = */ Red::AddressLib::EntityTemplate_FindAppearance,
    /* type = */ Red::TemplateAppearance* (*)(Red::EntityTemplate* aResource, Red::CName aName)>();
}
