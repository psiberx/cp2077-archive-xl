#include "Package.hpp"

Red::ObjectPackageHeader::ObjectPackageHeader()
    : rootIndex(-1)
{
}

bool Red::ObjectPackageHeader::IsEmpty() const
{
    return cruids.IsEmpty();
}

Red::ObjectPackageReader::ObjectPackageReader(void* aBuffer, uint32_t aSize)
{
    constexpr auto Construct = Core::RawFunc<
        /* addr = */ AddressLib::PackageLoader_ctor,
        /* func = */ void (*)(ObjectPackageReader*)>();

    Construct(this);

    buffer = aBuffer;
    size = aSize;
}

Red::ObjectPackageReader::ObjectPackageReader(const DeferredDataBuffer& aBuffer)
{
    constexpr auto Construct = Core::RawFunc<
        /* addr = */ AddressLib::PackageLoader_ctor,
        /* func = */ void (*)(ObjectPackageReader*)>();

    Construct(this);

    buffer = aBuffer.raw->data;
    size = aBuffer.raw->size;
}

Red::ObjectPackageReader::ObjectPackageReader(const DataBuffer& aBuffer)
{
    constexpr auto Construct = Core::RawFunc<
        /* addr = */ AddressLib::PackageLoader_ctor,
        /* func = */ void (*)(ObjectPackageReader*)>();

    Construct(this);

    buffer = aBuffer.buffer.data;
    size = aBuffer.buffer.size;
}

Red::ObjectPackageReader::ObjectPackageReader(const RawBuffer& aBuffer)
{
    constexpr auto Construct = Core::RawFunc<
        /* addr = */ AddressLib::PackageLoader_ctor,
        /* func = */ void (*)(ObjectPackageReader*)>();

    Construct(this);

    buffer = aBuffer.data;
    size = aBuffer.size;
}

void Red::ObjectPackageReader::ReadHeader()
{
    constexpr auto LoadData = Core::RawFunc<
        /* addr = */ AddressLib::PackageLoader_LoadData,
        /* func = */ void* (*)(ObjectPackageReader*, ObjectPackageHeader&)>();

    LoadData(this, header);
}

void Red::ObjectPackageReader::ReadHeader(Red::ObjectPackageHeader& aData)
{
    constexpr auto LoadData = Core::RawFunc<
        /* addr = */ AddressLib::PackageLoader_LoadData,
        /* func = */ void* (*)(ObjectPackageReader*, ObjectPackageHeader&)>();

    LoadData(this, aData);
}

bool Red::ObjectPackageReader::IsEmpty() const
{
    return !buffer;
}

void Red::ObjectPackageReader::sub_08(uint64_t a1, uint64_t a2)
{
    using func_t = void (*)(ObjectPackageReader*, uint64_t, uint64_t);
    static UniversalRelocFunc<func_t> func(AddressLib::ObjectPackageLoader_sub_08);
    func(this, a1, a2);
}

Red::PackageExtractorParams::PackageExtractorParams(const Red::PackageHeader& aHeader)
    : header(aHeader)
    , loader(ResourceLoader::Get())
    , disablePostLoad(false)
    , disableImports(false)
    , disablePreInitialization(false)
    , unk63(0)
    , unk64(0)
    , unk78(0)
{
}

Red::PackageExtractorParams::PackageExtractorParams(const Red::ObjectPackageHeader& aHeader)
    : PackageExtractorParams(aHeader.package)
{
}

Red::PackageExtractor::PackageExtractor(const Red::PackageExtractorParams& aParams)
    : unk78(0)
    , loader(nullptr)
    , disablePostLoad(false)
    , disableImports(false)
    , disablePreInitialization(false)
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
