#pragma once

namespace Red
{
using EntityTemplate = ent::EntityTemplate;
using TemplateAppearance = ent::TemplateAppearance;
}

namespace Raw::EntityTemplate
{
constexpr auto FindAppearance = Core::RawFunc<
    /* addr = */ Red::AddressLib::EntityTemplate_FindAppearance,
    /* type = */ Red::TemplateAppearance* (*)(Red::EntityTemplate* aResource, Red::CName aName)>();
}
