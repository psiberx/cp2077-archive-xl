#pragma once

#include "Red/Entity.hpp"

namespace Red
{
using ComponentPredicate = FlexCallback<bool (*)(IComponent*)>;

struct EntityBuilderRequest : EntityInitializeRequest
{
    using AllocatorType = Memory::EngineAllocator;

    uint64_t unkD0;                                 // D0
    WeakHandle<entEntityInstanceData> instanceData; // D8
    ComponentPredicate componentPredicate;          // E8 - Called during entity assemble to select components
    uint64_t unk118;                                // 118
};
RED4EXT_ASSERT_SIZE(EntityBuilderRequest, 0x120);

struct EntityBuilderAppearance
{
    Handle<appearanceAppearanceDefinition> definition; // 00
    Handle<appearanceAppearanceResource> resource;     // 10
    SharedPtr<ObjectPackageExtractor> extractor;       // 20
};
RED4EXT_ASSERT_SIZE(EntityBuilderAppearance, 0x30);

struct EntityBuilder
{
    using AllocatorType = Memory::EntityResourceAllocator;

    struct Flags
    {
        uint8_t ExtractEntity : 1;              // 00
        uint8_t unk01 : 1;                      // 01
        uint8_t ExtractAppearance : 1;          // 02
        uint8_t ExtractAppearances : 1;         // 03
        uint8_t InitializeVisualComponents : 1; // 04
    };
    RED4EXT_ASSERT_SIZE(Flags, 0x1);

    WeakPtr<EntityBuilder> self;                       // 00
    SharedPtr<ObjectPackageExtractor> entityExtractor; // 10
    EntityBuilderAppearance appearance;                // 20
    uint8_t unk50[0x30];                               // 50
    DynArray<EntityBuilderAppearance> appearances;     // 80
    Handle<entEntityTemplate> entityTemplate;          // 90
    SharedPtr<EntityBuilderRequest> request;           // A0
    Handle<Entity> entity;                             // B0
    DynArray<Handle<IComponent>> components;           // C0
    DynArray<void*> unkD0;                             // D0
    DynArray<void*> unkE0;                             // E0
    SharedSpinLock lock;                               // F0
    redTagList visualTags;                             // F8
    uint64_t unk108;                                   // 108
    Flags flags;                                       // 110
};
RED4EXT_ASSERT_SIZE(EntityBuilder, 0x118);
RED4EXT_ASSERT_OFFSET(EntityBuilder, appearance, 0x20);
RED4EXT_ASSERT_OFFSET(EntityBuilder, appearances, 0x80);
RED4EXT_ASSERT_OFFSET(EntityBuilder, entityTemplate, 0x90);
RED4EXT_ASSERT_OFFSET(EntityBuilder, request, 0xA0);
RED4EXT_ASSERT_OFFSET(EntityBuilder, components, 0xC0);
RED4EXT_ASSERT_OFFSET(EntityBuilder, visualTags, 0xF8);
RED4EXT_ASSERT_OFFSET(EntityBuilder, flags, 0x110);

struct EntityBuilderJobParams
{
    EntityBuilder* entityBuilder;
    WeakPtr<EntityBuilder> entityBuilderWeak;
};
}

namespace Raw::EntityBuilder
{
// using Template = Core::OffsetPtr<0x90, Red::Handle<Red::EntityTemplate>>;
// using Entity = Core::OffsetPtr<0xB0, Red::Handle<Red::Entity>>;
// using Components = Core::OffsetPtr<0xC0, Red::DynArray<Red::Handle<Red::IComponent>>>;
// using VisualTags = Core::OffsetPtr<0xF8, Red::redTagList>;
// using Flags = Core::OffsetPtr<0x110, uint8_t>;

constexpr auto ExtractComponentsJob = Core::RawFunc<
    /* addr = */ Red::AddressLib::EntityBuilder_ExtractComponentsJob,
    /* type = */ void (*)(Red::EntityBuilderJobParams* aParams, void* a2)>();

constexpr auto ScheduleExtractComponentsJob = Core::RawFunc<
    /* addr = */ Red::AddressLib::EntityBuilder_ScheduleExtractComponentsJob,
    /* type = */ void (*)(Red::JobQueue& aJobQueue, void* a2, Red::EntityBuilderJobParams* aParams)>();

constexpr auto ReleaseSharedPtr = Core::RawFunc<
    /* addr = */ Red::AddressLib::EntityBuilder_ReleaseSharedPtr,
    /* type = */ void (*)(Red::SharedPtr<Red::EntityBuilder>* aBuilder)>();
}

namespace Red
{
// Keeps an entity builder alive for the duration of a job, like the engine's own extraction job does.
// The reference is released by the engine, since the last owner must destroy the builder
// with the real destructor rather than with the partial layout declared above.
class EntityBuilderLock
{
public:
    explicit EntityBuilderLock(const WeakPtr<EntityBuilder>& aBuilderWeak) noexcept
        : m_builder(aBuilderWeak.Lock())
    {
    }

    ~EntityBuilderLock() noexcept
    {
        if (m_builder.refCount)
        {
            Raw::EntityBuilder::ReleaseSharedPtr(&m_builder);

            m_builder.instance = nullptr;
            m_builder.refCount = nullptr;
        }
    }

    EntityBuilderLock(const EntityBuilderLock&) = delete;
    EntityBuilderLock& operator=(const EntityBuilderLock&) = delete;

    explicit operator bool() const noexcept
    {
        return m_builder.instance != nullptr;
    }

    EntityBuilder* operator->() const noexcept
    {
        return m_builder.instance;
    }

private:
    SharedPtr<EntityBuilder> m_builder;
};
}
