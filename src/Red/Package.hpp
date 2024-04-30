#pragma once

#include "Red/Common.hpp"

namespace Red
{
struct PackageContent
{
    uint8_t unk00[0x58]{}; // 00
};
RED4EXT_ASSERT_SIZE(PackageContent, 0x58);

struct PackageData
{
    PackageData();

    [[nodiscard]] bool IsEmpty() const;

    uint16_t unk00;         // 00
    Range<uint64_t> unk08;  // 08
    PackageContent content; // 18
};
RED4EXT_ASSERT_SIZE(PackageData, 0x70);
RED4EXT_ASSERT_OFFSET(PackageData, unk08, 0x08);
RED4EXT_ASSERT_OFFSET(PackageData, content, 0x18);

struct PackageLoader
{
    PackageLoader(void* aBuffer, uint32_t aSize);
    PackageLoader(const DeferredDataBuffer& aBuffer);
    PackageLoader(const DataBuffer& aBuffer);
    PackageLoader(const RawBuffer& aBuffer);

    virtual ~PackageLoader() = default; // 00
    // virtual void sub_08(uint64_t a1, void* a2); // 08

    void Load();
    void Load(PackageData& aData);

    [[nodiscard]] bool IsEmpty() const;

    void* buffer;
    uint32_t size;
    PackageData data;
};
RED4EXT_ASSERT_SIZE(PackageLoader, 0x88);
RED4EXT_ASSERT_OFFSET(PackageLoader, buffer, 0x08);
RED4EXT_ASSERT_OFFSET(PackageLoader, size, 0x10);
RED4EXT_ASSERT_OFFSET(PackageLoader, data, 0x18);

struct PackageExtractorParams
{
    PackageExtractorParams(const PackageData& aData);
    PackageExtractorParams(const PackageContent& aContent);

    PackageContent content;   // 00
    ResourceLoader* loader;   // 58
    uint8_t unk60;            // 60
    uint8_t unk61;            // 61
    uint8_t unk62;            // 62
    uint8_t unk63;            // 63
    uint8_t unk64;            // 64
    DynArray<uint32_t> unk68; // 68
    uint32_t unk78;           // 78
    RawBuffer unk80;          // 80
};
RED4EXT_ASSERT_SIZE(PackageExtractorParams, 0xA0);
RED4EXT_ASSERT_OFFSET(PackageExtractorParams, loader, 0x58);
RED4EXT_ASSERT_OFFSET(PackageExtractorParams, unk68, 0x68);
RED4EXT_ASSERT_OFFSET(PackageExtractorParams, unk80, 0x80);

struct PackageExtractor
{
    PackageExtractor(const PackageExtractorParams& aParams);

    void ExtractSync();
    JobHandle ExtractAsync();

    WeakPtr<PackageExtractor> self;                 // 00
    PackageContent content;                         // 10
    DynArray<uint32_t> unk68;                       // 68
    uint32_t unk78;                                 // 78
    RawBuffer unk80;                                // 80
    ResourceLoader* loader;                         // A0
    DynArray<Handle<ISerializable>> results;        // A8
    uint8_t unkB8;                                  // B8
    bool disableImports;                            // B9
    uint8_t unkBA;                                  // BA
    uint8_t unkBB;                                  // BB
    uint8_t unkBC;                                  // BC
    DynArray<Handle<ISerializable>> objects;        // C0
    DynArray<SharedPtr<ResourceToken<>>> resources; // D0
    DynArray<uint32_t> unkE0;                       // E0
    void* unkF0;                                    // F0
    void* unkF8;                                    // F8
    void* unk100;                                   // 100
    void* unk108;                                   // 108
    void* unk110;                                   // 110
    void* unk118;                                   // 118
};
RED4EXT_ASSERT_SIZE(PackageExtractor, 0x120);
}
