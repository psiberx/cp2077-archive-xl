#pragma once

namespace Raw::Entity
{
// using EntityID = Core::OffsetPtr<0x48, Red::EntityID>;
// using AppearanceName = Core::OffsetPtr<0x50, Red::CName>;
// using TemplatePath = Core::OffsetPtr<0x60, Red::ResourcePath>;
// using ComponentsStorage = Core::OffsetPtr<0x70, Red::ent::ComponentsStorage>;
using VisualTags = Core::OffsetPtr<0x138, Red::TagList>;
using EntityTags = Core::OffsetPtr<0x230, Red::TagList>;

constexpr auto ReassembleAppearance = Core::RawFunc<
    /* addr = */ Red::AddressLib::Entity_ReassembleAppearance,
    /* type = */ void (*)(Red::Entity*, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>();
}

namespace Raw::IComponent
{
// using AppearanceName = Core::OffsetPtr<0x48, Red::CName>;
// using Owner = Core::OffsetPtr<0x50, Red::Entity*>;
// using AppearancePath = Core::OffsetPtr<0x68, Red::ResourcePath>;

constexpr auto UpdateRenderer = Core::RawVFunc<
    /* offset = */ 0x280,
    /* type = */ void (Red::IComponent::*)()>();
}
