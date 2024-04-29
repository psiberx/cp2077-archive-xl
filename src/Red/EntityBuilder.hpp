#pragma once

namespace Raw::EntityBuilder
{
using Template = Core::OffsetPtr<0x90, Red::Handle<Red::entEntityTemplate>>;
using Entity = Core::OffsetPtr<0xD0, Red::Handle<Red::Entity>>;
using Components = Core::OffsetPtr<0xC0, Red::DynArray<Red::Handle<Red::IComponent>>>;
using VisualTags = Core::OffsetPtr<0xF8, Red::redTagList>;
using Flags = Core::OffsetPtr<0x110, uint8_t>;

constexpr auto ExtractComponentsJob = Core::RawFunc<
    /* addr = */ Red::AddressLib::EntityBuilder_ExtractComponentsJob,
    /* type = */ void (*)(void** aBuilder, void* a2)>();
}
