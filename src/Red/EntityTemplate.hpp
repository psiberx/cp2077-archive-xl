#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Red
{
using EntityTemplate = ent::EntityTemplate;
using TemplateAppearance = ent::TemplateAppearance;
}

namespace Raw::EntityTemplate
{
constexpr auto FindAppearance = Core::RawFunc<
    /* addr = */ Red::Addresses::EntityTemplate_FindAppearance,
    /* type = */ Red::TemplateAppearance* (*)(Red::EntityTemplate* aResource, Red::CName aName)>();
}
