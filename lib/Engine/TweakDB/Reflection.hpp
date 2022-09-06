#pragma once

#include "Core/Stl.hpp"
#include "Engine/Stl.hpp"

#include <RED4ext/CName.hpp>
#include <RED4ext/RTTISystem.hpp>
#include <RED4ext/RTTITypes.hpp>
#include <RED4ext/TweakDB.hpp>

namespace TweakDB
{
class Reflection
{
public:
    struct PropertyInfo
    {
        RED4ext::CName name;
        const RED4ext::CBaseRTTIType* type;
        bool isArray;
        const RED4ext::CBaseRTTIType* elementType;
        bool isForeignKey;
        const RED4ext::CClass* foreignType;
        std::string appendix; // The name used to build TweakDBID of the property
        uintptr_t offset;
    };

    struct RecordInfo
    {
        RED4ext::CName name;
        const RED4ext::CClass* type;
        const RED4ext::CClass* parent;
        Core::SortedMap<RED4ext::CName, Core::SharedPtr<PropertyInfo>> props;
        std::string shortName;

        [[nodiscard]] const PropertyInfo* GetPropInfo(RED4ext::CName aPropName) const
        {
            const auto& propIt = props.find(aPropName);
            return propIt != props.end() ? propIt->second.get() : nullptr;
        }
    };

    Reflection();
    explicit Reflection(RED4ext::TweakDB* aTweakDb);

    const RED4ext::CBaseRTTIType* GetFlatType(RED4ext::CName aTypeName);
    const RED4ext::CClass* GetRecordType(RED4ext::CName aTypeName);
    const RED4ext::CClass* GetRecordType(const std::string& aTypeName);

    const RecordInfo* GetRecordInfo(RED4ext::CName aTypeName);
    const RecordInfo* GetRecordInfo(const RED4ext::CClass* aType);

private:
    Core::SharedPtr<TweakDB::Reflection::RecordInfo> CollectRecordInfo(const RED4ext::CClass* aType, RED4ext::TweakDBID aSampleId = 0);

    RED4ext::TweakDBID GetRecordSampleId(const RED4ext::CClass* aType);
    std::string ResolvePropertyName(RED4ext::TweakDBID aSampleId, RED4ext::CName aGetterName);

    RED4ext::TweakDB* m_tweakDb;
    RED4ext::CRTTISystem* m_rtti;
    Core::Map<RED4ext::CName, Core::SharedPtr<RecordInfo>> m_records;
};
}
