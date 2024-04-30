#include "Package.hpp"

Red::PackageData::PackageData()
    : unk00(-1)
{
}

bool Red::PackageData::IsEmpty() const
{
    return unk08.IsEmpty();
}

Red::PackageLoader::PackageLoader(void* aBuffer, uint32_t aSize)
{
    constexpr auto Construct = Core::RawFunc<
        /* addr = */ AddressLib::PackageLoader_ctor,
        /* func = */ void (*)(PackageLoader*)>();

    Construct(this);

    buffer = aBuffer;
    size = aSize;
}

Red::PackageLoader::PackageLoader(const DeferredDataBuffer& aBuffer)
{
    constexpr auto Construct = Core::RawFunc<
        /* addr = */ AddressLib::PackageLoader_ctor,
        /* func = */ void (*)(PackageLoader*)>();

    Construct(this);

    buffer = aBuffer.raw->data;
    size = aBuffer.raw->size;
}

Red::PackageLoader::PackageLoader(const DataBuffer& aBuffer)
{
    constexpr auto Construct = Core::RawFunc<
        /* addr = */ AddressLib::PackageLoader_ctor,
        /* func = */ void (*)(PackageLoader*)>();

    Construct(this);

    buffer = aBuffer.buffer.data;
    size = aBuffer.buffer.size;
}

Red::PackageLoader::PackageLoader(const RawBuffer& aBuffer)
{
    constexpr auto Construct = Core::RawFunc<
        /* addr = */ AddressLib::PackageLoader_ctor,
        /* func = */ void (*)(PackageLoader*)>();

    Construct(this);

    buffer = aBuffer.data;
    size = aBuffer.size;
}

void Red::PackageLoader::Load()
{
    constexpr auto LoadData = Core::RawFunc<
        /* addr = */ AddressLib::PackageLoader_LoadData,
        /* func = */ void* (*)(PackageLoader*, PackageData&)>();

    LoadData(this, data);
}

void Red::PackageLoader::Load(Red::PackageData& aData)
{
    constexpr auto LoadData = Core::RawFunc<
        /* addr = */ AddressLib::PackageLoader_LoadData,
        /* func = */ void* (*)(PackageLoader*, PackageData&)>();

    LoadData(this, aData);
}

bool Red::PackageLoader::IsEmpty() const
{
    return !buffer;
}

Red::PackageExtractorParams::PackageExtractorParams(const Red::PackageContent& aContent)
    : content(aContent)
    , loader(ResourceLoader::Get())
    , unk60(0)
    , unk61(0)
    , unk62(0)
    , unk63(0)
    , unk64(0)
    , unk78(0)
{
}

Red::PackageExtractorParams::PackageExtractorParams(const Red::PackageData& aData)
    : PackageExtractorParams(aData.content)
{
}

Red::PackageExtractor::PackageExtractor(const Red::PackageExtractorParams& aParams)
    : unk78(0)
    , loader(nullptr)
    , unkB8(0)
    , disableImports(false)
    , unkBA(0)
    , unkBB(0)
    , unkBC(0)
    , unkF0(nullptr)
    , unkF8(nullptr)
    , unk100(nullptr)
    , unk108(nullptr)
    , unk110(nullptr)
    , unk118(nullptr)
{
    constexpr auto Initialize = Core::RawFunc<
        /* addr = */ AddressLib::PackageExtractor_Initialize,
        /* func = */ void (*)(PackageExtractor*, const PackageExtractorParams&)>();

    Initialize(this, aParams);
}

void Red::PackageExtractor::ExtractSync()
{
    constexpr auto ExtractSync = Core::RawFunc<
        /* addr = */ AddressLib::PackageExtractor_ExtractSync,
        /* func = */void (*)(PackageExtractor*)>();

    ExtractSync(this);
}

RED4ext::JobHandle Red::PackageExtractor::ExtractAsync()
{
    constexpr auto ExtractAsync = Core::RawFunc<
        /* addr = */ AddressLib::PackageExtractor_ExtractAsync,
        /* func = */ void* (*)(PackageExtractor*, JobHandle&)>();

    JobHandle job;
    ExtractAsync(this, job);

    return job;
}
