#pragma once

namespace Red
{
using QuestNodeID = uint16_t;
using QuestNodePath = DynArray<QuestNodeID>;
using QuestNodePathHash = uint32_t;

inline bool IsRelatedQuestNodePath(const QuestNodePath& aParentPath, const QuestNodePath& aPath)
{
    return aPath.size > aParentPath.size &&
           std::memcmp(aPath.entries, aParentPath.entries, aParentPath.size * sizeof(Red::QuestNodeID)) == 0;
}

inline QuestNodePathHash MakeQuestNodePathHash(const QuestNodePath& aNodePath, int32_t aLength = 0)
{
    if (aLength == 0)
    {
        aLength = static_cast<int32_t>(aNodePath.size);
    }
    else if (aLength < 0)
    {
        aLength += static_cast<int32_t>(aNodePath.size);
    }

    return FNV1a32(reinterpret_cast<uint8_t*>(aNodePath.entries), aLength * sizeof(QuestNodeID));
}

struct QuestNodeKey
{
    QuestNodeKey() = default;

    QuestNodeKey(QuestNodePathHash aPhaseHash, QuestNodeID aNodeID = 0)
        : phaseHash(aPhaseHash)
        , nodeID(aNodeID)
    {
    }

    QuestNodeKey(const QuestNodePath& aNodePath)
        : phaseHash(MakeQuestNodePathHash(aNodePath, -1))
        , nodeID(aNodePath.Back())
    {
    }

    QuestNodeKey(const QuestNodePath& aNodePath, QuestNodeID aNodeID)
        : phaseHash(MakeQuestNodePathHash(aNodePath))
        , nodeID(aNodeID)
    {
    }

    operator uint64_t() const noexcept
    {
        return (static_cast<uint64_t>(phaseHash) << 32) + nodeID;
    }

    operator uint32_t() const noexcept
    {
        uint64_t data = *this;
        return FNV1a32(reinterpret_cast<uint8_t*>(&data), sizeof(data));
    }

    QuestNodePathHash phaseHash;
    QuestNodePathHash nodeID;
};
RED4EXT_ASSERT_SIZE(QuestNodeKey, 0x8);
RED4EXT_ASSERT_OFFSET(QuestNodeKey, phaseHash, 0x0);
RED4EXT_ASSERT_OFFSET(QuestNodeKey, nodeID, 0x4);

struct FactID
{
    constexpr FactID(uint32_t aHash = 0) noexcept
        : hash(aHash)
    {
    }

    constexpr FactID(const char* aName) noexcept
        : hash(FNV1a32(aName))
    {
    }

    FactID(QuestNodeKey aPath) noexcept
        : hash(aPath)
    {
    }

    operator uint32_t() const noexcept
    {
        return hash;
    }

    uint32_t hash;
};

struct FactStore
{
    uint64_t unk00;
    Red::Map<FactID, int32_t> data;
};

using FactStoreIndex = uint32_t;

struct FactManager
{
    static constexpr FactStoreIndex NamedFactStore = 1;
    static constexpr FactStoreIndex GraphStoreMin = 2;
    static constexpr FactStoreIndex GraphStoreMax = 10;

    virtual ~FactManager() = 0;                                                    // 00
    virtual void sub_08() = 0;                                                     // 08
    virtual int32_t GetFact(FactStoreIndex aStore, FactID aFact) = 0;              // 10
    virtual void SetFact(FactStoreIndex aStore, FactID aFact, int32_t aValue) = 0; // 18

    inline int32_t GetFact(FactID aFact)
    {
        return GetFact(NamedFactStore, aFact);
    }

    inline void SetFact(FactID aFact, int32_t aValue)
    {
        SetFact(NamedFactStore, aFact, aValue);
    }

    inline void ResetFact(FactID aFact)
    {
        SetFact(NamedFactStore, aFact, 0);
    }

    inline void ResetGraphFact(FactID aFact)
    {
        for (auto store = GraphStoreMin; store <= GraphStoreMax; ++store)
        {
            SetFact(store, aFact, 0);
        }
    }

    FactStore* data[11];
};

struct QuestPhaseContext
{
    virtual ~QuestPhaseContext() = default;

    void* game;                  // 08
    void* unk10;                 // 10
    QuestsSystem* questsSystem;  // 18
    void* unk20;                 // 20
    void* unk28;                 // 28
    void* prefabLoader;          // 30
    uint8_t unk38[0x230 - 0x38]; // 38
};
RED4EXT_ASSERT_SIZE(QuestPhaseContext, 0x230);
RED4EXT_ASSERT_OFFSET(QuestPhaseContext, questsSystem, 0x18);

struct QuestContext
{
    uint8_t unk0[0xF8];                       // 00
    DynArray<questPhaseInstance*> phaseStack; // F8
    QuestNodeID nodeID;                       // 108
    QuestPhaseContext phaseContext;           // 110
};
RED4EXT_ASSERT_SIZE(QuestContext, 0x340);
RED4EXT_ASSERT_OFFSET(QuestContext, phaseStack, 0xF8);
}

namespace Raw::QuestsSystem
{
using Mutex = Core::OffsetPtr<0x60, Red::SharedMutex>;
using NodeHashMap = Core::OffsetPtr<0x78, Red::HashMap<Red::QuestNodePathHash, Red::QuestNodePath>>;
using QuestList = Core::OffsetPtr<0xA8, Red::DynArray<Red::ResourcePath>>;
using FactManager = Core::OffsetPtr<0xF8, Red::FactManager*>;

constexpr auto OnGameRestored = Core::RawFunc<
    /* addr = */ Red::AddressLib::QuestsSystem_OnGameRestored,
    /* type = */ void (*)(Red::QuestsSystem* aSystem)>();

constexpr auto ForceStartNode = Core::RawVFunc<
    /* addr = */ 0x240,
    /* type = */ void (Red::questIQuestsSystem::*)(const Red::QuestNodeKey& aNodeKey,
                                                   const Red::DynArray<Red::CName>& aInputSockets)>();
}

namespace Raw::QuestRootInstance
{
constexpr auto Start = Core::RawFunc<
    /* addr = */ Red::AddressLib::QuestRootInstance_Start,
    /* type = */ void (*)(Red::questRootInstance* aInstance,
                          Red::QuestContext* aContext,
                          const Red::Handle<Red::questQuestResource>& aResource)>();
}
