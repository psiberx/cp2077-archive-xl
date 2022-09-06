#include "Reflection.hpp"
#include "Types.hpp"

#include <RED4ext/Scripting/Functions.hpp>
#include <RED4ext/Scripting/CProperty.hpp>

namespace
{
constexpr size_t DataOffsetSize = 3;
}

TweakDB::Reflection::Reflection()
    : Reflection(RED4ext::TweakDB::Get())
{
}

TweakDB::Reflection::Reflection(RED4ext::TweakDB* aTweakDb)
    : m_rtti(RED4ext::CRTTISystem::Get())
    , m_tweakDb(aTweakDb)
{
}

const RED4ext::CBaseRTTIType* TweakDB::Reflection::GetFlatType(RED4ext::CName aTypeName)
{
    const RED4ext::CBaseRTTIType* type = m_rtti->GetType(aTypeName);

    if (!RTDB::IsFlatType(type))
        return nullptr;

    return type;
}

const RED4ext::CClass* TweakDB::Reflection::GetRecordType(RED4ext::CName aTypeName)
{
    return GetRecordType(std::string(aTypeName.ToString()));
}

const RED4ext::CClass* TweakDB::Reflection::GetRecordType(const std::string& aTypeName)
{
    auto aFullName = RTDB::GetRecordFullName(aTypeName);

    RED4ext::CClass* type = m_rtti->GetClass(aFullName);

    if (!RTDB::IsRecordType(type))
        return nullptr;

    return type;
}

const TweakDB::Reflection::RecordInfo* TweakDB::Reflection::GetRecordInfo(const RED4ext::CClass* aType)
{
    if (!RTDB::IsRecordType(aType))
        return nullptr;

    auto iter = m_records.find(aType->GetName());

    if (iter != m_records.end())
        return iter->second.get();

    return CollectRecordInfo(aType).get();
}

const TweakDB::Reflection::RecordInfo* TweakDB::Reflection::GetRecordInfo(RED4ext::CName aTypeName)
{
    auto iter = m_records.find(aTypeName);

    if (iter != m_records.end())
        return iter->second.get();

    return CollectRecordInfo(m_rtti->GetClass(aTypeName)).get();
}

Core::SharedPtr<TweakDB::Reflection::RecordInfo> TweakDB::Reflection::CollectRecordInfo(const RED4ext::CClass* aType,
                                                                                        RED4ext::TweakDBID aSampleId)
{
    if (!RTDB::IsRecordType(aType))
        return nullptr;

    auto sampleId = aSampleId;
    if (!sampleId.IsValid())
    {
        sampleId = GetRecordSampleId(aType);

        if (!sampleId.IsValid())
            return nullptr;
    }

    auto recordInfo = Core::MakeShared<RecordInfo>();
    recordInfo->name = aType->name;
    recordInfo->type = aType;
    recordInfo->shortName = RTDB::GetRecordShortName(aType->name);

    const auto parentInfo = CollectRecordInfo(aType->parent, sampleId);
    if (parentInfo)
    {
        recordInfo->parent = aType->parent;
        recordInfo->props.insert(parentInfo->props.begin(), parentInfo->props.end());
    }

    const auto baseOffset = aType->parent->size;

    for (uint32_t funcIndex = 0u; funcIndex < aType->funcs.size; ++funcIndex)
    {
        const auto func = aType->funcs[funcIndex];

        auto propName = ResolvePropertyName(sampleId, func->shortName);

        auto propInfo = Core::MakeShared<PropertyInfo>();
        propInfo->name = RED4ext::CName(propName.c_str());
        propInfo->offset = baseOffset + DataOffsetSize * recordInfo->props.size();

        // Case: Foreign Key Array => TweakDBID[]
        if (!func->returnType)
        {
            const auto arrayType = reinterpret_cast<RED4ext::CRTTIArrayType*>(func->params[0]->type);
            const auto handleType = reinterpret_cast<RED4ext::CRTTIWeakHandleType*>(arrayType->innerType);
            const auto recordType = reinterpret_cast<RED4ext::CClass*>(handleType->innerType);

            propInfo->type = m_rtti->GetType(RTDB::EFlatType::TweakDBIDArray);
            propInfo->isArray = true;
            propInfo->elementType = m_rtti->GetType(RTDB::EFlatType::TweakDBID);
            propInfo->isForeignKey = true;
            propInfo->foreignType = recordType;

            // Skip related functions:
            // func Get[Prop]Count()
            // func Get[Prop]Item()
            // func Get[Prop]ItemHandle()
            // func [Prop]Contains()
            funcIndex += 4;
        }
        else
        {
            auto returnType = func->returnType->type;

            switch (returnType->GetType())
            {
            case RED4ext::ERTTIType::WeakHandle:
            {
                // Case: Foreign Key => TweakDBID
                const auto handleType = reinterpret_cast<RED4ext::CRTTIWeakHandleType*>(returnType);
                const auto recordType = reinterpret_cast<RED4ext::CClass*>(handleType->innerType);

                propInfo->type = m_rtti->GetType(RTDB::EFlatType::TweakDBID);
                propInfo->isForeignKey = true;
                propInfo->foreignType = recordType;

                // Skip related function:
                // func Get[Prop]Handle()
                funcIndex += 1;
                break;
            }
            case RED4ext::ERTTIType::Array:
            {
                if (RTDB::IsResRefTokenArray(returnType))
                {
                    propInfo->type = m_rtti->GetType(TweakDB::RTDB::EFlatType::ResourceArray);
                    propInfo->isArray = true;
                    propInfo->elementType = m_rtti->GetType(RTDB::EFlatType::Resource);

                    // Skip related functions:
                    // func Get[Prop]Count()
                    // func Get[Prop]Item()
                    funcIndex += 2;
                }
                else
                {
                    const auto arrayType = reinterpret_cast<RED4ext::CRTTIArrayType*>(returnType);
                    const auto elementType = reinterpret_cast<RED4ext::CBaseRTTIType*>(arrayType->innerType);

                    propInfo->type = returnType;
                    propInfo->isArray = true;
                    propInfo->elementType = elementType;

                    // Skip related functions:
                    // func Get[Prop]Count()
                    // func Get[Prop]Item()
                    // func [Prop]Contains()
                    funcIndex += 3;
                }
                break;
            }
            case RED4ext::ERTTIType::Enum:
            {
                // Some types have additional enum getters,
                // but they're not backed by any flat.
                continue;
            }
            default:
            {
                if (RTDB::IsResRefToken(returnType))
                {
                    propInfo->type = m_rtti->GetType(TweakDB::RTDB::EFlatType::Resource);
                }
                else
                {
                    // Getter for LocKey returns CName, so we have to get
                    // the actual property type from the flat value.
                    if (returnType->GetType() == RED4ext::ERTTIType::Name)
                    {
                        static std::string dot = ".";
                        auto propId = sampleId + dot + propName;
                        auto flat = m_tweakDb->GetFlatValue(propId);
                        returnType = flat->GetValue().type;
                    }

                    propInfo->type = returnType;
                }
            }
            }
        }

        assert(propInfo->type);

        propInfo->appendix = ".";
        propInfo->appendix.append(propName);

        recordInfo->props.insert({ propInfo->name, propInfo });
    }

    m_records.insert({ recordInfo->name, recordInfo });

    return recordInfo;
}

RED4ext::TweakDBID TweakDB::Reflection::GetRecordSampleId(const RED4ext::CClass* aType)
{
    std::shared_lock<RED4ext::SharedMutex> recordLockR(m_tweakDb->mutex01);
    auto* records = m_tweakDb->recordsByType.Get(const_cast<RED4ext::CClass*>(aType));

    if (records == nullptr)
        return {};

    return reinterpret_cast<RED4ext::Handle<RED4ext::gamedataTweakDBRecord>*>(records->Begin())->GetPtr()->recordID;
}

std::string TweakDB::Reflection::ResolvePropertyName(RED4ext::TweakDBID aSampleId, RED4ext::CName aGetterName)
{
    static std::string dot = ".";

    std::string propName = aGetterName.ToString();
    propName[0] = std::tolower(propName[0]);

    auto propId = aSampleId + dot + propName;

    std::shared_lock<RED4ext::SharedMutex> flatLockR(m_tweakDb->mutex00);

    auto flat = m_tweakDb->flats.Find(propId);

    flatLockR.unlock();

    if (flat == m_tweakDb->flats.End())
        propName[0] = std::toupper(propName[0]);

    return propName;
}
