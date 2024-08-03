#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "App/Extensions/ResourcePatch/Config.hpp"
#include "Red/AppearanceResource.hpp"
#include "Red/EntityBuilder.hpp"
#include "Red/EntityTemplate.hpp"
#include "Red/GarmentAssembler.hpp"
#include "Red/Serialization.hpp"

namespace App
{
class ResourcePatchModule : public ConfigurableModuleImpl<ResourcePatchConfig>
{
public:
    std::string_view GetName() override;
    bool Load() override;
    bool Unload() override;
    void Configure() override;

private:
    using DefinitionMap = Core::Map<Red::CName, Red::WeakHandle<Red::AppearanceDefinition>>;

    static void OnResourceRequest(Red::ResourceDepot*, const uintptr_t* aOut, Red::ResourcePath aPath, const int32_t*);
    static void OnResourceDeserialize(void* aSerializer, uint64_t, uint64_t, Red::JobHandle& aJob,
                                      Red::ResourceSerializerRequest& aRequest, uint64_t,
                                      Red::DynArray<Red::Handle<Red::ISerializable>>&, uint64_t);
    static void OnEntityTemplateLoad(Red::EntityTemplate* aTemplate, void*);
    static void OnAppearanceResourceLoad(Red::AppearanceResource* aResource);
    static void OnMeshResourceLoad(Red::CMesh* aMesh, void*);
    static void OnEntityPackageExtract(Red::EntityBuilderJobParams* aParams, void*);
    static void OnPartPackageExtract(Red::DynArray<Red::Handle<Red::ISerializable>>& aResultObjects,
                                     const Red::SharedPtr<Red::ResourceToken<Red::EntityTemplate>>& aPartToken);
    static void OnGarmentPackageExtract(Red::GarmentComponentParams* aParams, const Red::JobGroup& aJobGroup);

    static void PatchPackageExtractorResults(const Red::Handle<Red::EntityTemplate>& aTemplate,
                                      Red::DynArray<Red::Handle<Red::ISerializable>>& aResultObjects);
    static void PatchPackageExtractorResults(const Red::Handle<Red::AppearanceResource>& aResource,
                                      const Red::Handle<Red::AppearanceDefinition>& aDefinition,
                                      Red::DynArray<Red::Handle<Red::ISerializable>>& aResultObjects);
    static void PatchResultEntity(Red::DynArray<Red::Handle<Red::ISerializable>>& aResultObjects,
                                  Red::DynArray<Red::Handle<Red::ISerializable>>& aPatchObjects, int16_t aEntityIndex);
    static void PatchResultComponents(Red::DynArray<Red::Handle<Red::ISerializable>>& aResultObjects,
                                      Red::DynArray<Red::Handle<Red::ISerializable>>& aPatchObjects);

    static const Core::Set<Red::ResourcePath>& GetPatchList(Red::ResourcePath aTargetPath);
    template<typename T = Red::CResource>
    static Red::Handle<T> GetPatchResource(Red::ResourcePath aPatchPath);
    template<typename T = Red::CResource>
    static Red::SharedPtr<Red::ResourceToken<T>> GetPatchToken(Red::ResourcePath aPatchPath);
    static Red::Handle<Red::AppearanceDefinition> GetPatchDefinition(Red::ResourcePath aResourcePath,
                                                                     Red::CName aDefinitionName);

    inline static Core::Map<Red::ResourcePath, Core::Set<Red::ResourcePath>> s_patches;
    inline static Core::Map<Red::ResourcePath, std::string> s_paths;

    inline static Red::SharedSpinLock s_tokenLock;
    inline static Core::Map<Red::ResourcePath, Red::SharedPtr<Red::ResourceToken<>>> s_tokens;

    inline static Red::SharedSpinLock s_definitionLock;
    inline static Core::Map<Red::ResourcePath, DefinitionMap> s_definitions;
};
}
