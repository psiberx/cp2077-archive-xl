#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Raw::Entity
{
constexpr auto GetComponents = Core::RawFunc<
    /* addr = */ Red::Addresses::Entity_GetComponents,
    /* type = */ Red::DynArray<Red::Handle<Red::ent::IComponent>>& (*)(Red::ent::Entity*)>();

constexpr auto ReassembleAppearance = Core::RawFunc<
    /* addr = */ Red::Addresses::Entity_ReassembleAppearance,
    /* type = */ void (*)(Red::ent::Entity*, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>();
}
