#pragma once

namespace Red
{
template<typename T>
inline TweakDBID CreateFlat(TweakDBID aRecordID, const char* aProp, T&& aValue)
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
inline void AppendToFlat(TweakDBID aRecordID, const char* aProp, const Core::Vector<T>& aValues)
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
        if (!copy.Contains(value))
        {
            copy.PushBack(value);
        }
    }

    const auto offset = tweakDB->CreateFlatValue({data.type, &copy});
    flatID.SetTDBOffset(offset);

    {
        std::unique_lock _(tweakDB->mutex00);
        tweakDB->flats.InsertOrAssign(flatID);
    }
}

inline TweakDBID CreateRecord(TweakDBID aRecordID, const char* aType)
{
    auto tweakDB = TweakDB::Get();

    const auto hash = Red::Murmur3_32(reinterpret_cast<const uint8_t*>(aType), strlen(aType));

    using CreateTDBRecord_t = void (*)(TweakDB*, uint32_t, TweakDBID);
    static UniversalRelocFunc<CreateTDBRecord_t> CreateTDBRecord(RED4ext::Detail::AddressHashes::TweakDB_CreateRecord);
    CreateTDBRecord(tweakDB, hash, aRecordID);

    return aRecordID;
}

inline bool CreateRecordAlias(TweakDBID aRecordID, TweakDBID aAliasID)
{
    auto tweakDB = TweakDB::Get();

    std::unique_lock _(tweakDB->mutex01);

    auto record = tweakDB->recordsByID.Get(aRecordID);

    if (!record)
        return false;

    tweakDB->recordsByID.Insert(aAliasID, *record);
    return true;
}

inline void UpdateRecord(TweakDBID aRecordID)
{
    auto tweakDB = TweakDB::Get();

    tweakDB->UpdateRecord(aRecordID);
}

inline bool RecordExists(TweakDBID aRecordID)
{
    auto tweakDB = Red::TweakDB::Get();

    std::shared_lock _(tweakDB->mutex01);

    return tweakDB->recordsByID.Get(aRecordID);
}

template<typename T>
inline T* GetFlatPtr(TweakDBID aFlatID)
{
    auto tweakDB = Red::TweakDB::Get();

    std::shared_lock _(tweakDB->mutex00);

    auto* flatValue = tweakDB->GetFlatValue(aFlatID);

    if (flatValue == nullptr)
        return nullptr;

    return flatValue->GetValue<T>();
}

template<typename T>
inline T GetFlatValue(TweakDBID aFlatID)
{
    auto* flatPtr = GetFlatPtr<T>(aFlatID);

    if (flatPtr == nullptr)
        return {};

    return *flatPtr;
}

inline CString ToStringDebug(TweakDBID aID)
{
    Red::CString str;
    Red::CallStatic("gamedataTDBIDHelper", "ToStringDEBUG", str, aID);
    return str;
}
}

namespace Raw
{
constexpr auto LoadTweakDB = Core::RawFunc<
    /* addr = */ Red::AddressLib::TweakDB_Load,
    /* type = */ void (*)(Red::TweakDB*, Red::CString&)>();
}
