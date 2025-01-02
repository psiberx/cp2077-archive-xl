#pragma once

#include "App/Extensions/ExtensionBase.hpp"
#include "App/Extensions/ResourcePatch/Config.hpp"
#include "Red/AppearanceResource.hpp"
#include "Red/EntityBuilder.hpp"
#include "Red/EntityTemplate.hpp"
#include "Red/GarmentAssembler.hpp"
#include "Red/Serialization.hpp"

namespace App
{
class ResourcePatchExtension : public ConfigurableExtensionImpl<ResourcePatchConfig>
{
public:
    std::string_view GetName() override;
    bool Load() override;
    bool Unload() override;
    void Configure() override;

private:
    using DefinitionData = std::pair<Red::Handle<Red::AppearanceDefinition>, Red::SharedPtr<Red::DeferredDataBufferToken>>;
    using DefinitionMap = Core::Map<Red::CName, DefinitionData>;

    static void OnResourceRequest(Red::ResourceDepot*, const uintptr_t* aOut, Red::ResourcePath aPath, const int32_t*);
    static void OnResourceDeserialize(void* aSerializer, uint64_t, uint64_t, Red::JobHandle& aJob,
                                      Red::ResourceSerializerRequest& aRequest, uint64_t,
                                      Red::DynArray<Red::Handle<Red::ISerializable>>&, uint64_t);
    static void OnResourceReady(Red::ResourceSerializer* aSerializer);
    static void OnEntityTemplateLoad(Red::EntityTemplate* aTemplate, void*);
    static void OnAppearanceResourceLoad(Red::AppearanceResource* aResource);
    static void OnMeshResourceLoad(Red::CMesh* aMesh, Red::PostLoadParams* aParams);
    static void OnMorphTargetResourceLoad(Red::MorphTargetMesh* aMorphTarget, Red::PostLoadParams* aParams);
    static void OnEntityPackageLoad(Red::JobQueue& aJobQueue, void*, Red::EntityBuilderJobParams* aParams);
    static void OnPartPackageExtract(Red::DynArray<Red::Handle<Red::ISerializable>>& aResultObjects,
                                     const Red::SharedPtr<Red::ResourceToken<Red::EntityTemplate>>& aPartToken);
    static void OnGarmentPackageExtract(Red::GarmentExtractionParams* aParams, const Red::JobGroup& aJobGroup);
    static void OnCurveSetResourceLoad(Red::CurveSet* aResource);

    static void IncludeAppearanceParts(const Red::Handle<Red::AppearanceResource>& aResource,
                                       const Red::Handle<Red::AppearanceDefinition>& aDefinition,
                                       Red::DynArray<Red::Handle<Red::ISerializable>>& aResultObjects,
                                       bool aDisablePostLoad, bool aDisableImports, bool aDisablePreInitialization,
                                       bool aForceIncludeParts, const Red::JobGroup& aJobGroup);

    static void PatchPackageResults(const Red::Handle<Red::EntityTemplate>& aTemplate,
                                    Red::DynArray<Red::Handle<Red::ISerializable>>& aResultObjects,
                                    bool aDisablePostLoad,
                                    bool aDisableImports,
                                    bool aDisablePreInitialization);
    static void PatchPackageResults(const Red::Handle<Red::AppearanceResource>& aResource,
                                    const Red::Handle<Red::AppearanceDefinition>& aDefinition,
                                    Red::DynArray<Red::Handle<Red::ISerializable>>& aResultObjects,
                                    bool aDisablePostLoad,
                                    bool aDisableImports,
                                    bool aDisablePreInitialization,
                                    const Red::JobGroup& aJobGroup);

    static void MergeEntity(Red::DynArray<Red::Handle<Red::ISerializable>>& aResultObjects,
                            Red::DynArray<Red::Handle<Red::ISerializable>>& aPatchObjects,
                            int16_t aResultEntityIndex, int16_t aPatchEntityIndex);
    static void MergeComponents(Red::DynArray<Red::Handle<Red::ISerializable>>& aResultObjects,
                                Red::DynArray<Red::Handle<Red::ISerializable>>& aPatchObjects);
    static void MergeObjects(Red::DynArray<Red::Handle<Red::ISerializable>>& aResultObjects,
                             Red::DynArray<Red::Handle<Red::ISerializable>>& aPatchObjects);
    static void MergeResources(Red::DynArray<Red::SharedPtr<Red::ResourceToken<>>>& aResultResources,
                               Red::DynArray<Red::SharedPtr<Red::ResourceToken<>>>& aPatchResources);

    inline static Red::Handle<Red::rendRenderMeshBlob> CopyRenderBlob(
        const Red::Handle<Red::rendRenderMeshBlob>& aSourceBlob);
    inline static Red::Handle<Red::rendRenderMorphTargetMeshBlob> CopyRenderBlob(
        const Red::Handle<Red::rendRenderMorphTargetMeshBlob>& aSourceBlob);

    static const Core::Set<Red::ResourcePath>& GetPatchList(Red::ResourcePath aTargetPath);
    template<typename T = Red::CResource>
    static Red::Handle<T> GetPatchResource(Red::ResourcePath aPatchPath);
    template<typename T = Red::CResource>
    static Red::SharedPtr<Red::ResourceToken<T>> GetPatchToken(Red::ResourcePath aPatchPath);
    static Red::Handle<Red::AppearanceDefinition> GetPatchDefinition(Red::ResourcePath aResourcePath, Red::CName aDefinitionName);

    inline static Core::Map<Red::ResourcePath, Core::Set<Red::ResourcePath>> s_patches;
    inline static Core::Map<Red::ResourcePath, std::string> s_paths;
    inline static Red::SharedSpinLock s_tokenLock;
    inline static Core::Map<Red::ResourcePath, Red::SharedPtr<Red::ResourceToken<>>> s_tokens;
    inline static Red::SharedSpinLock s_definitionLock;
    inline static Core::Map<Red::ResourcePath, DefinitionMap> s_definitions;
};
}
