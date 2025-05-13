#pragma once

#include "App/Extensions/ExtensionBase.hpp"
#include "App/Extensions/Garment/Dynamic.hpp"
#include "App/Shared/ResourcePathRegistry.hpp"
#include "Red/Mesh.hpp"

namespace App
{
class MeshExtension : public Extension
{
public:
    bool Load() override;
    bool Unload() override;
    std::string_view GetName() override;

    static bool IsContextualMesh(Red::CMesh* aMesh);
    static bool IsSpecialMaterial(Red::CName aMaterialName);
    static void PrefetchMeshState(Red::CMesh* aMesh, const Core::Map<Red::CName, std::string>& aContext);
    static Red::CName RegisterMeshSource(Red::CMesh* aMesh, Red::CMesh* aSourceMesh);

private:
    struct MeshState
    {
        MeshState(Red::CMesh* aMesh);

        void MarkStatic();
        [[nodiscard]] bool IsStatic() const;

        void PrefetchContext(Red::CMesh* aMesh);
        void PrefillContext(const Core::Map<Red::CName, std::string>& aContext);
        void ResolveContextProperties();
        void EnsureContextReady();
        const Red::DynArray<Red::MaterialParameterInstance>& GetContextParams();
        const DynamicAttributeList& GetContextAttrs();
        std::string_view GetContextAttr(Red::CName aAttr);

        Red::CName GetDefaultExpansionName();
        int32_t GetExpansionIndex(Red::CName aExpansionName);

        void FillAppearances(Red::CMesh* aMesh);
        void FillMaterials(Red::CMesh* aMesh);
        void RegisterMaterialEntry(Red::CName aMaterialName, int32_t aEntryIndex);
        [[nodiscard]] int32_t GetTemplateEntryIndex(Red::CName aMaterialName);
        [[nodiscard]] int32_t GetMaterialEntryIndex(Red::CName aMaterialName);
        [[nodiscard]] bool HasMaterialEntry(Red::CName aMaterialName) const;

        Red::CName RegisterSource(Red::CMesh* aSourceMesh);
        [[nodiscard]] Red::Handle<Red::CMesh> ResolveSource(Red::CName aSourceName);

        volatile bool dynamic;
        Red::ResourcePath meshPath;
        Red::SharedSpinLock meshMutex;
        Red::SharedSpinLock sourceMutex;
        Red::JobHandle lastJob;
        Red::SharedPtr<Red::ResourceToken<Red::IMaterial>> contextToken;
        Red::DynArray<Red::MaterialParameterInstance> contextParams;
        DynamicAttributeList contextAttrs;
        Red::CName expansionSource;
        Core::Map<Red::CName, int32_t> appearances;
        Core::Map<Red::CName, int32_t> templates;
        Core::Map<Red::CName, int32_t> materials;
        Core::Map<Red::CName, Red::WeakHandle<Red::CMesh>> sources;
    };

    struct ChunkData
    {
        Red::CName chunkName{};
        Red::CName materialName{};
        Red::CName templateName{};
        int32_t sourceIndex{-1};
        Red::SharedPtr<Red::ResourceToken<Red::IMaterial>> sourceToken{};
        Red::Handle<Red::CMaterialInstance> sourceInstance{};
        bool failed{false};
    };

    struct DynamicContext
    {
        Core::SharedPtr<MeshState> targetState;
        Core::SharedPtr<MeshState> sourceState;
        Red::Handle<Red::CMesh> targetMesh;
        Red::Handle<Red::CMesh> sourceMesh;
        Red::DynArray<Red::CName> materialNames;
        Red::SharedPtr<Red::DynArray<Red::Handle<Red::IMaterial>>> finalMaterials;
        Core::Vector<Core::SharedPtr<ChunkData>> deferredMaterials{};
    };

    static void OnFindAppearance(Red::Handle<Red::mesh::MeshAppearance>& aAppearance, Red::CMesh* aMesh, Red::CName aName);
    static void* OnLoadMaterials(Red::CMesh* aTargetMesh, Red::MeshMaterialsToken& aToken,
                                 const Red::DynArray<Red::CName>& aMaterialNames, uint8_t a4);
    static void OnAddStubAppearance(Red::CMesh* aMesh);
    static bool OnPreloadAppearances(Red::CMesh* aMesh);

    static void ProcessDynamicMaterials(const Core::SharedPtr<DynamicContext>& aContext,
                                        const Red::JobGroup& aJobGroup);
    static void FinalizeDynamicMaterial(const Core::SharedPtr<DynamicContext>& aContext,
                                        const Core::SharedPtr<ChunkData>& aChunk,
                                        Red::JobQueue& aJobQueue);
    static void FixFinalMaterialsIndexes(const Core::SharedPtr<DynamicContext>& aContext);
    static Red::Handle<Red::CMaterialInstance> CloneMaterialInstance(
        const Core::SharedPtr<DynamicContext>& aContext, const Core::SharedPtr<ChunkData>& aChunk,
        const Red::Handle<Red::CMaterialInstance>& aSourceInstance, Red::JobQueue& aJobQueue);
    static void ExpandMaterialInheritance(const Core::SharedPtr<DynamicContext>& aContext,
                                                  const Core::SharedPtr<ChunkData>& aChunk,
                                                  const Red::Handle<Red::CMaterialInstance>& aMaterialInstance,
                                                  Red::JobQueue& aJobQueue);
    static void ExpandMaterialParams(const Core::SharedPtr<DynamicContext>& aContext,
                                             const Core::SharedPtr<ChunkData>& aChunk,
                                             const Red::Handle<Red::CMaterialInstance>& aMaterialInstance,
                                             Red::JobQueue& aJobQueue);
    static std::pair<Red::ResourcePath, bool> ExpandResourcePath(Red::ResourcePath aPath, Red::CName aMaterialName,
                                                                 const Core::SharedPtr<MeshState>& aState);
    static bool ContainsUnresolvedMaterials(const Red::DynArray<Red::Handle<Red::IMaterial>>& aMaterials);

    static Core::SharedPtr<MeshState> AcquireMeshState(Red::CMesh* aMesh);
    static Core::SharedPtr<MeshState> FindMeshState(uint64_t aHash);

    inline static Core::Map<Red::ResourcePath, Core::SharedPtr<MeshState>> s_states;
    inline static Red::Handle<Red::CMesh> s_dummyMesh;
    inline static Red::Handle<Red::meshMeshAppearance> s_dummyAppearance;
    inline static Red::Handle<Red::CMaterialInstance> s_dummyMaterial;
    inline static Red::SharedSpinLock s_stateLock;
    inline static Core::SharedPtr<ResourcePathRegistry> s_resourcePathRegistry;
};
}
