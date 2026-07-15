#pragma once

#include "App/Extensions/ExtensionBase.hpp"
#include "App/Extensions/ResourcePatch/Config.hpp"
#include "App/Shared/ResourcePathRegistry.hpp"
#include "Red/AppearanceResource.hpp"
#include "Red/Common.hpp"
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

    static bool IsPatched(const Red::Handle<Red::meshMeshAppearance>& aAppearance);
    static Red::CName GetExpansionName(const Red::Handle<Red::meshMeshAppearance>& aAppearance);
    static Red::CName GetPatchSource(const Red::Handle<Red::meshMeshAppearance>& aAppearance);

    static void RegisterPatch(Red::ResourcePath aTargetPath, Red::ResourcePath aPatchPath);
    static void RegisterPatch(Red::ResourcePath aTargetPath, const char* aPatchPathStr);
    // static void ClearTarget(Red::ResourcePath aTarget);

private:
    struct PatchInstance
    {
        PatchInstance(Red::ResourcePath aSource);
        PatchInstance(Red::ResourcePath aSource, Core::Set<Red::ResourcePath> aTargets, Core::Set<Red::CName> aProps,
                      int32_t aOrder);

        [[nodiscard]] bool Modifies(Red::CName aProp) const;
        [[nodiscard]] bool Modifies(Red::CName aProp, bool aOverwrite) const;

        void LoadResource();
        template<typename T = Red::CResource>
        Red::Handle<T> GetResource() const;
        template<typename T = Red::CResource>
        Red::ResourceTokenPtr<T> GetToken() const;

        void PrefetchAppearance(const Red::Handle<Red::AppearanceDefinition>& aDefinition);
        Red::Handle<Red::AppearanceDefinition> GetAppearanceDefinition(Red::CName aName);

        Red::ResourcePath path;
        Core::Set<Red::CName> props;
        Core::Set<Red::ResourcePath> targets;
        int32_t order{0};

        Red::SharedPtr<Red::ResourceToken<>> token;
        Core::Map<Red::CName, Red::Handle<Red::AppearanceDefinition>> appearances;
        Red::SharedSpinLock appearanceLock;
    };

    using PatchInstancePtr = Core::SharedPtr<PatchInstance>;

    static void OnResourceRequest(void* aSerializer, uint64_t a2, Red::ResourceSerializerRequest& aRequest,
                                  uint64_t a4, uint64_t a5);
    static void OnResourceDeserialize(void* aSerializer, uint64_t, uint64_t, Red::JobHandle& aJob,
                                      Red::ResourceSerializerRequest& aRequest, uint64_t,
                                      Red::DynArray<Red::Handle<Red::ISerializable>>&, uint64_t);
    static void OnResourceReady(Red::ResourceSerializerContext* aContext);
    static void OnEntityTemplateLoad(Red::EntityTemplate* aTemplate, void*);
    static void OnAppearanceResourceLoad(Red::AppearanceResource* aResource, Red::PostLoadParams*);
    static void OnMeshResourceLoad(Red::CMesh* aMesh, Red::PostLoadParams* aParams);
    static void OnMorphTargetResourceLoad(Red::MorphTargetMesh* aMorphTarget, Red::PostLoadParams* aParams);
    static void OnEntityPackageLoad(Red::JobQueue& aJobQueue, void*, Red::EntityBuilderJobParams* aParams);
    static void OnPartPackageExtract(Red::DynArray<Red::Handle<Red::ISerializable>>& aResultObjects,
                                     const Red::SharedPtr<Red::ResourceToken<Red::EntityTemplate>>& aPartToken);
    static void OnGarmentPackageExtract(Red::GarmentExtractionParams* aParams, const Red::JobGroup& aJobGroup);
    static void OnStreamingWorldLoad(Red::worldStreamingWorld* aWorld);
    static void OnCurveSetResourceLoad(Red::CurveSet* aResource);
    static void OnDeviceResourceLoad(Red::gameDeviceResource* aResource);
    static void OnSetPersistentStateData(uint64_t a1, Red::DataBuffer& aData, uint64_t a3, uint32_t a4);

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
                                Red::DynArray<Red::Handle<Red::ISerializable>>& aPatchObjects,
                                bool aPartMerge, Red::ResourcePath aPatchPath, Red::ResourcePath aTargetPath,
                                Red::CName aPatchDefinition = {}, Red::CName aTargetDefinition = {});

    static Red::Handle<Red::rendRenderMeshBlob> CopyRenderBlob(const Red::Handle<Red::rendRenderMeshBlob>& aSourceBlob);
    static Red::Handle<Red::rendRenderMorphTargetMeshBlob> CopyRenderBlob(
        const Red::Handle<Red::rendRenderMorphTargetMeshBlob>& aSourceBlob);

    static const Core::Vector<PatchInstancePtr>& GetPatchList(Red::ResourcePath aTargetPath);
    static bool IsPatchResource(Red::ResourcePath aPath);

    inline static Core::Map<Red::ResourcePath, Core::Vector<PatchInstancePtr>> s_patches;
    inline static Core::Map<Red::ResourcePath, Core::Vector<PatchInstancePtr>> s_targetPatches;
    inline static Core::Map<Red::ResourcePath, PatchInstancePtr> s_dynamicPatches;
    inline static Core::SharedPtr<ResourcePathRegistry> s_resourcePathRegistry;
};
}
