#pragma once

#include "Red/Addresses.hpp"

namespace Red
{
template<typename T>
TweakDBID CreateFlat(TweakDBID aRecordID, const char* aProp, T&& aValue)
{
    auto tweakDB = TweakDB::Get();

    const auto type = GetType<T>();
    const auto offset = tweakDB->CreateFlatValue({type, &aValue});

    auto flatID = TweakDBID(aRecordID, aProp);
    flatID.SetTDBOffset(offset);

    tweakDB->AddFlat(flatID);

    return flatID;
}

template<typename T>
void AppendToFlat(TweakDBID aRecordID, const char* aProp, const Core::Vector<T>& aValues)
{
    auto tweakDB = TweakDB::Get();
    auto flatID = TweakDBID(aRecordID, aProp);

    auto flat = tweakDB->GetFlatValue(flatID);
    if (!flat)
        return;

    auto data = flat->GetValue();

    if (data.type->GetType() != ERTTIType::Array)
        return;

    auto copy = *reinterpret_cast<DynArray<T>*>(data.value);
    for (const auto& value : aValues)
    {
        // TODO: Check for duplicates
        copy.PushBack(value);
    }

    const auto offset = tweakDB->CreateFlatValue({data.type, &copy});
    flatID.SetTDBOffset(offset);

    {
        std::lock_guard<SharedMutex> _(tweakDB->mutex00);
        tweakDB->flats.InsertOrAssign(flatID);
    }
}

TweakDBID CreateRecord(TweakDBID aRecordID, const char* aType)
{
    auto tweakDB = TweakDB::Get();

    const auto hash = Red::Murmur3_32(reinterpret_cast<const uint8_t*>(aType), strlen(aType));

    using CreateTDBRecord_t = void (*)(TweakDB*, uint32_t, TweakDBID);
    RelocFunc<CreateTDBRecord_t> CreateTDBRecord(RED4ext::Addresses::TweakDB_CreateRecord);
    CreateTDBRecord(tweakDB, hash, aRecordID);

    return aRecordID;
}

void UpdateRecord(TweakDBID aRecordID)
{
    auto tweakDB = TweakDB::Get();

    tweakDB->UpdateRecord(aRecordID);
}

bool RecordExists(TweakDBID aRecordID)
{
    auto tweakDB = Red::TweakDB::Get();

    std::shared_lock _(tweakDB->mutex01);

    return tweakDB->recordsByID.Get(aRecordID);
}
}

namespace Raw
{
constexpr auto LoadTweakDB = Core::RawFunc<
    /* addr = */ Red::Addresses::TweakDB_Load,
    /* type = */ void (*)(Red::TweakDB*, Red::CString&)>();
}
