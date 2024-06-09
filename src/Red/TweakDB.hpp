#pragma once

namespace Red
{
template<typename T>
inline TweakDBID CreateFlat(TweakDBID aFlatID, T&& aValue)
{
    auto tweakDB = TweakDB::Get();

    const auto type = GetType<T>();
    const auto offset = tweakDB->CreateFlatValue({type, &aValue});

    aFlatID.SetTDBOffset(offset);
    tweakDB->AddFlat(aFlatID);

    return aFlatID;
}

template<typename T>
inline TweakDBID CreateFlat(TweakDBID aRecordID, const char* aProp, T&& aValue)
{
    return CreateFlat({aRecordID, aProp}, std::forward<T>(aValue));
}

template<typename I, typename T = std::remove_const_t<typename std::iterator_traits<I>::value_type>>
inline void AppendToFlat(TweakDBID aFlatID, const I& aBegin, const I& aEnd)
{
    auto tweakDB = TweakDB::Get();

    auto flat = tweakDB->GetFlatValue(aFlatID);
    if (!flat)
        return;

    auto data = flat->GetValue();
    if (data.type->GetType() != ERTTIType::Array)
        return;

    auto copy = *reinterpret_cast<DynArray<T>*>(data.value);
    for (auto value = aBegin; value != aEnd; value++)
    {
        if (!copy.Contains(*value))
        {
            copy.PushBack(*value);
        }
    }

    const auto offset = tweakDB->CreateFlatValue({data.type, &copy});
    aFlatID.SetTDBOffset(offset);

    {
        std::unique_lock _(tweakDB->mutex00);
        tweakDB->flats.InsertOrAssign(aFlatID);
    }
}

template<typename I, typename T = std::remove_const_t<typename std::iterator_traits<I>::value_type>>
inline void AppendToFlat(TweakDBID aRecordID, const char* aProp, const I& aBegin, const I& aEnd)
{
    AppendToFlat({aRecordID, aProp}, aBegin, aEnd);
}

template<typename V>
inline void AppendToFlat(TweakDBID aFlatID, const V& aValues)
{
    AppendToFlat(aFlatID, aValues.begin(), aValues.end());
}

template<typename V>
inline void AppendToFlat(TweakDBID aRecordID, const char* aProp, const V& aValues)
{
    AppendToFlat({aRecordID, aProp}, aValues.begin(), aValues.end());
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
