#include "Types.hpp"

#include <RED4ext/NativeTypes.hpp>
#include <RED4ext/ResourceReference.hpp>
#include <RED4ext/RTTISystem.hpp>
#include <RED4ext/Scripting/Natives/Generated/Color.hpp>
#include <RED4ext/Scripting/Natives/Generated/EulerAngles.hpp>
#include <RED4ext/Scripting/Natives/Generated/Vector2.hpp>
#include <RED4ext/Scripting/Natives/Generated/Vector3.hpp>
#include <RED4ext/Scripting/Natives/Generated/Quaternion.hpp>

namespace
{
constexpr RED4ext::CName BaseRecordTypeName = "gamedataTweakDBRecord";
constexpr RED4ext::CName ResRefTokenTypeName = "redResourceReferenceScriptToken";
constexpr RED4ext::CName ResRefTokenArrayTypeName = "array:redResourceReferenceScriptToken";
constexpr const char* RecordTypePrefix = "gamedata";
constexpr const char* RecordTypeSuffix = "_Record";
constexpr size_t RecordTypePrefixLength = std::char_traits<char>::length(RecordTypePrefix);
constexpr size_t RecordTypeSuffixLength = std::char_traits<char>::length(RecordTypeSuffix);
}

bool TweakDB::RTDB::IsFlatType(RED4ext::CName aTypeName)
{
    switch (aTypeName)
    {
    case EFlatType::Int32:
    case EFlatType::Float:
    case EFlatType::Bool:
    case EFlatType::String:
    case EFlatType::CName:
    case EFlatType::TweakDBID:
    case EFlatType::LocKey:
    case EFlatType::Resource:
    case EFlatType::Quaternion:
    case EFlatType::EulerAngles:
    case EFlatType::Vector3:
    case EFlatType::Vector2:
    case EFlatType::Color:
    case EFlatType::Int32Array:
    case EFlatType::FloatArray:
    case EFlatType::BoolArray:
    case EFlatType::StringArray:
    case EFlatType::CNameArray:
    case EFlatType::TweakDBIDArray:
    case EFlatType::LocKeyArray:
    case EFlatType::ResourceArray:
    case EFlatType::QuaternionArray:
    case EFlatType::EulerAnglesArray:
    case EFlatType::Vector3Array:
    case EFlatType::Vector2Array:
    case EFlatType::ColorArray:
        return true;
    default:
        return false;
    }
}

bool TweakDB::RTDB::IsFlatType(const RED4ext::CBaseRTTIType* aType)
{
    return aType && IsFlatType(aType->GetName());
}

bool TweakDB::RTDB::IsRecordType(RED4ext::CName aTypeName)
{
    return IsRecordType(RED4ext::CRTTISystem::Get()->GetClass(aTypeName));
}

bool TweakDB::RTDB::IsRecordType(const RED4ext::CClass* aType)
{
    static RED4ext::CBaseRTTIType* s_baseRecordType = RED4ext::CRTTISystem::Get()->GetClass(BaseRecordTypeName);

    return aType && aType != s_baseRecordType && aType->IsA(s_baseRecordType);
}

bool TweakDB::RTDB::IsArrayType(RED4ext::CName aTypeName)
{
    switch (aTypeName)
    {
    case EFlatType::Int32Array:
    case EFlatType::FloatArray:
    case EFlatType::BoolArray:
    case EFlatType::StringArray:
    case EFlatType::CNameArray:
    case EFlatType::TweakDBIDArray:
    case EFlatType::LocKeyArray:
    case EFlatType::ResourceArray:
    case EFlatType::QuaternionArray:
    case EFlatType::EulerAnglesArray:
    case EFlatType::Vector3Array:
    case EFlatType::Vector2Array:
    case EFlatType::ColorArray:
        return true;
    default:
        return false;
    }
}

bool TweakDB::RTDB::IsArrayType(const RED4ext::CBaseRTTIType* aType)
{
    return aType && IsArrayType(aType->GetName());
}

bool TweakDB::RTDB::IsForeignKey(RED4ext::CName aTypeName)
{
    return aTypeName == EFlatType::TweakDBID;
}

bool TweakDB::RTDB::IsForeignKey(const RED4ext::CBaseRTTIType* aType)
{
    return aType && IsForeignKey(aType->GetName());
}

bool TweakDB::RTDB::IsForeignKeyArray(RED4ext::CName aTypeName)
{
    return aTypeName == EFlatType::TweakDBIDArray;
}

bool TweakDB::RTDB::IsForeignKeyArray(const RED4ext::CBaseRTTIType* aType)
{
    return aType && IsForeignKeyArray(aType->GetName());
}

bool TweakDB::RTDB::IsResRefToken(RED4ext::CName aTypeName)
{
    return aTypeName == ResRefTokenTypeName;
}

bool TweakDB::RTDB::IsResRefToken(const RED4ext::CBaseRTTIType* aType)
{
    return aType && IsResRefToken(aType->GetName());
}

bool TweakDB::RTDB::IsResRefTokenArray(RED4ext::CName aTypeName)
{
    return aTypeName == ResRefTokenArrayTypeName;
}

bool TweakDB::RTDB::IsResRefTokenArray(const RED4ext::CBaseRTTIType* aType)
{
    return aType && IsResRefTokenArray(aType->GetName());
}

RED4ext::CName TweakDB::RTDB::GetArrayType(RED4ext::CName aTypeName)
{
    switch (aTypeName)
    {
    case EFlatType::Int32: return EFlatType::Int32Array;
    case EFlatType::Float: return EFlatType::FloatArray;
    case EFlatType::Bool: return EFlatType::BoolArray;
    case EFlatType::String: return EFlatType::StringArray;
    case EFlatType::CName: return EFlatType::CNameArray;
    case EFlatType::TweakDBID: return EFlatType::TweakDBIDArray;
    case EFlatType::LocKey: return EFlatType::LocKeyArray;
    case EFlatType::Resource: return EFlatType::ResourceArray;
    case EFlatType::Quaternion: return EFlatType::QuaternionArray;
    case EFlatType::EulerAngles: return EFlatType::EulerAnglesArray;
    case EFlatType::Vector3: return EFlatType::Vector3Array;
    case EFlatType::Vector2: return EFlatType::Vector2Array;
    case EFlatType::Color: return EFlatType::ColorArray;
    }

    return {};
}

RED4ext::CName TweakDB::RTDB::GetArrayType(const RED4ext::CBaseRTTIType* aType)
{
    if (!aType)
        return {};

    return GetArrayType(aType->GetName());
}

RED4ext::CName TweakDB::RTDB::GetElementType(RED4ext::CName aTypeName)
{
    switch (aTypeName)
    {
    case EFlatType::Int32Array: return EFlatType::Int32;
    case EFlatType::FloatArray: return EFlatType::Float;
    case EFlatType::BoolArray: return EFlatType::Bool;
    case EFlatType::StringArray: return EFlatType::String;
    case EFlatType::CNameArray: return EFlatType::CName;
    case EFlatType::TweakDBIDArray: return EFlatType::TweakDBID;
    case EFlatType::LocKeyArray: return EFlatType::LocKey;
    case EFlatType::ResourceArray: return EFlatType::Resource;
    case EFlatType::QuaternionArray: return EFlatType::Quaternion;
    case EFlatType::EulerAnglesArray: return EFlatType::EulerAngles;
    case EFlatType::Vector3Array: return EFlatType::Vector3;
    case EFlatType::Vector2Array: return EFlatType::Vector2;
    case EFlatType::ColorArray: return EFlatType::Color;
    }

    return {};
}

RED4ext::CName TweakDB::RTDB::GetElementType(const RED4ext::CBaseRTTIType* aType)
{
    if (!aType)
        return {};

    return GetElementType(aType->GetName());
}

RED4ext::CName TweakDB::RTDB::GetRecordFullName(RED4ext::CName aName)
{
    return GetRecordFullName(std::string(aName.ToString()));
}

RED4ext::CName TweakDB::RTDB::GetRecordFullName(const std::string& aName)
{
    std::string finalName = aName;

    if (!finalName.starts_with(RecordTypePrefix))
        finalName.insert(0, RecordTypePrefix);

    if (!finalName.ends_with(RecordTypeSuffix))
        finalName.append(RecordTypeSuffix);

    return finalName.c_str();
}

std::string TweakDB::RTDB::GetRecordShortName(RED4ext::CName aName)
{
    return GetRecordShortName(std::string(aName.ToString()));
}

std::string TweakDB::RTDB::GetRecordShortName(const std::string& aName)
{
    std::string finalName = aName;

    if (finalName.starts_with(RecordTypePrefix))
        finalName.erase(0, RecordTypePrefixLength);

    if (finalName.ends_with(RecordTypeSuffix))
        finalName.erase(finalName.end() - RecordTypeSuffixLength, finalName.end());

    return finalName;
}

Core::SharedPtr<void> TweakDB::RTDB::MakeDefaultValue(RED4ext::CName aTypeName)
{
    switch (aTypeName)
    {
    case EFlatType::Int32: return MakeDefaultValue<int>();
    case EFlatType::Float: return MakeDefaultValue<float>();
    case EFlatType::Bool: return MakeDefaultValue<bool>();
    case EFlatType::String: return MakeDefaultValue<RED4ext::CString>();
    case EFlatType::CName: return MakeDefaultValue<RED4ext::CName>();
    case EFlatType::TweakDBID: return MakeDefaultValue<RED4ext::TweakDBID>();
    case EFlatType::LocKey: return MakeDefaultValue<RED4ext::gamedataLocKeyWrapper>();
    case EFlatType::Resource: return MakeDefaultValue<RED4ext::ResourceAsyncReference<>>();
    case EFlatType::Quaternion: return MakeDefaultValue<RED4ext::Quaternion>();
    case EFlatType::EulerAngles: return MakeDefaultValue<RED4ext::EulerAngles>();
    case EFlatType::Vector3: return MakeDefaultValue<RED4ext::Vector3>();
    case EFlatType::Vector2: return MakeDefaultValue<RED4ext::Vector2>();
    case EFlatType::Color: return MakeDefaultValue<RED4ext::Color>();
    case EFlatType::Int32Array: return MakeDefaultValue<RED4ext::DynArray<int>>();
    case EFlatType::FloatArray: return MakeDefaultValue<RED4ext::DynArray<float>>();
    case EFlatType::BoolArray: return MakeDefaultValue<RED4ext::DynArray<bool>>();
    case EFlatType::StringArray: return MakeDefaultValue<RED4ext::DynArray<RED4ext::CString>>();
    case EFlatType::CNameArray: return MakeDefaultValue<RED4ext::DynArray<RED4ext::CName>>();
    case EFlatType::TweakDBIDArray: return MakeDefaultValue<RED4ext::DynArray<RED4ext::TweakDBID>>();
    case EFlatType::LocKeyArray: return MakeDefaultValue<RED4ext::DynArray<RED4ext::gamedataLocKeyWrapper>>();
    case EFlatType::ResourceArray: return MakeDefaultValue<RED4ext::DynArray<RED4ext::ResourceAsyncReference<>>>();
    case EFlatType::QuaternionArray: return MakeDefaultValue<RED4ext::DynArray<RED4ext::Quaternion>>();
    case EFlatType::EulerAnglesArray: return MakeDefaultValue<RED4ext::DynArray<RED4ext::EulerAngles>>();
    case EFlatType::Vector3Array: return MakeDefaultValue<RED4ext::DynArray<RED4ext::Vector3>>();
    case EFlatType::Vector2Array: return MakeDefaultValue<RED4ext::DynArray<RED4ext::Vector2>>();
    case EFlatType::ColorArray: return MakeDefaultValue<RED4ext::DynArray<RED4ext::Color>>();
    }

    return nullptr;
}

Core::SharedPtr<void> TweakDB::RTDB::MakeDefaultValue(const RED4ext::CBaseRTTIType* aType)
{
    if (!aType)
        return {};

    return MakeDefaultValue(aType->GetName());
}

