#pragma once

#include "Core/Stl.hpp"

#include <RED4ext/CName.hpp>
#include <RED4ext/RTTITypes.hpp>

namespace TweakDB::RTDB
{
namespace EFlatType
{
enum Name: uint64_t
{
    Int32 = (uint64_t) RED4ext::CName("Int32"),
    Float = (uint64_t) RED4ext::CName("Float"),
    Bool = (uint64_t) RED4ext::CName("Bool"),
    String = (uint64_t) RED4ext::CName("String"),
    CName = (uint64_t) RED4ext::CName("CName"),
    TweakDBID = (uint64_t) RED4ext::CName("TweakDBID"),
    LocKey = (uint64_t) RED4ext::CName("gamedataLocKeyWrapper"),
    Resource = (uint64_t) RED4ext::CName("raRef:CResource"),
    Quaternion = (uint64_t) RED4ext::CName("Quaternion"),
    EulerAngles = (uint64_t) RED4ext::CName("EulerAngles"),
    Vector3 = (uint64_t) RED4ext::CName("Vector3"),
    Vector2 = (uint64_t) RED4ext::CName("Vector2"),
    Color = (uint64_t) RED4ext::CName("Color"),
    Int32Array = (uint64_t) RED4ext::CName("array:Int32"),
    FloatArray = (uint64_t) RED4ext::CName("array:Float"),
    BoolArray = (uint64_t) RED4ext::CName("array:Bool"),
    StringArray = (uint64_t) RED4ext::CName("array:String"),
    CNameArray = (uint64_t) RED4ext::CName("array:CName"),
    TweakDBIDArray = (uint64_t) RED4ext::CName("array:TweakDBID"),
    LocKeyArray = (uint64_t) RED4ext::CName("array:gamedataLocKeyWrapper"),
    ResourceArray = (uint64_t) RED4ext::CName("array:raRef:CResource"),
    QuaternionArray = (uint64_t) RED4ext::CName("array:Quaternion"),
    EulerAnglesArray = (uint64_t) RED4ext::CName("array:EulerAngles"),
    Vector3Array = (uint64_t) RED4ext::CName("array:Vector3"),
    Vector2Array = (uint64_t) RED4ext::CName("array:Vector2"),
    ColorArray = (uint64_t) RED4ext::CName("array:Color"),
};
}

bool IsFlatType(RED4ext::CName aTypeName);
bool IsFlatType(const RED4ext::CBaseRTTIType* aType);

bool IsRecordType(RED4ext::CName aTypeName);
bool IsRecordType(const RED4ext::CClass* aType);

bool IsArrayType(RED4ext::CName aTypeName);
bool IsArrayType(const RED4ext::CBaseRTTIType* aType);

bool IsForeignKey(RED4ext::CName aTypeName);
bool IsForeignKey(const RED4ext::CBaseRTTIType* aType);

bool IsForeignKeyArray(RED4ext::CName aTypeName);
bool IsForeignKeyArray(const RED4ext::CBaseRTTIType* aType);

bool IsResRefToken(RED4ext::CName aTypeName);
bool IsResRefToken(const RED4ext::CBaseRTTIType* aType);

bool IsResRefTokenArray(RED4ext::CName aTypeName);
bool IsResRefTokenArray(const RED4ext::CBaseRTTIType* aType);

RED4ext::CName GetArrayType(RED4ext::CName aTypeName);
RED4ext::CName GetArrayType(const RED4ext::CBaseRTTIType* aType);

RED4ext::CName GetElementType(RED4ext::CName aTypeName);
RED4ext::CName GetElementType(const RED4ext::CBaseRTTIType* aType);

RED4ext::CName GetRecordFullName(RED4ext::CName aName);
RED4ext::CName GetRecordFullName(const std::string& aName);

std::string GetRecordShortName(RED4ext::CName aName);
std::string GetRecordShortName(const std::string& aName);

Core::SharedPtr<void> MakeDefaultValue(RED4ext::CName aTypeName);
Core::SharedPtr<void> MakeDefaultValue(const RED4ext::CBaseRTTIType* aType);

template<typename T>
Core::SharedPtr<T> MakeDefaultValue()
{
    return Core::MakeShared<T>();
}
}
