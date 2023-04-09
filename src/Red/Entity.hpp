#pragma once

#include "Red/Addresses.hpp"

namespace Raw::Entity
{
using ComponentsStorage = Core::OffsetPtr<0x70, Red::ent::ComponentsStorage>;
using VisualTags = Core::OffsetPtr<0x138, Red::TagList>;
using Tags = Core::OffsetPtr<0x230, Red::TagList>;

constexpr auto GetComponents = Core::RawFunc<
    /* addr = */ Red::Addresses::Entity_GetComponents,
    /* type = */ Red::DynArray<Red::Handle<Red::ent::IComponent>>& (*)(Red::ent::Entity*)>();

constexpr auto ReassembleAppearance = Core::RawFunc<
    /* addr = */ Red::Addresses::Entity_ReassembleAppearance,
    /* type = */ void (*)(Red::ent::Entity*, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>();
}
