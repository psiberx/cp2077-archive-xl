#pragma once

#include "App/Extensions/ExtensionBase.hpp"
#include "App/Extensions/Garment/Dynamic.hpp"
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
        void FillContext(const Core::Map<Red::CName, std::string>& aContext);
        void EnsureContextFilled();
        const Red::DynArray<Red::MaterialParameterInstance>& GetContextParams();
        const DynamicAttributeList& GetContextAttrs();

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
        Core::Map<Red::CName, int32_t> templates;
        Core::Map<Red::CName, int32_t> materials;
        Core::Map<Red::CName, Red::WeakHandle<Red::CMesh>> sources;
    };

    struct DeferredMaterial
    {
        int32_t chunkIndex;
        Red::CName chunkName;
        Red::CName materialName;
        Red::CName templateName;
        int32_t sourceIndex;
        Red::SharedPtr<Red::ResourceToken<Red::IMaterial>> sourceToken;
    };

    static void OnFindAppearance(Red::Handle<Red::mesh::MeshAppearance>& aOut, Red::CMesh* aMesh, Red::CName aName);
    static void* OnLoadMaterials(Red::CMesh* aTargetMesh, Red::MeshMaterialsToken& aToken,
                                 const Red::DynArray<Red::CName>& aMaterialNames, uint8_t a4);
    static void OnAddStubAppearance(Red::CMesh* aMesh);
    static bool OnPreloadAppearances(Red::CMesh* aMesh);

    static void ProcessMeshResource(const Core::SharedPtr<MeshState>& aMeshState,
                                    const Red::Handle<Red::CMesh>& aMeshWeak,
                                    const Core::SharedPtr<MeshState>& aSourceState,
                                    const Red::Handle<Red::CMesh>& aSourceMeshWeak,
                                    const Red::DynArray<Red::CName>& aMaterialNames,
                                    const Red::SharedPtr<Red::DynArray<Red::Handle<Red::IMaterial>>>& aFinalMaterials,
                                    const Red::JobGroup& aJobGroup);
    static bool ContainsUnresolvedMaterials(const Red::DynArray<Red::Handle<Red::IMaterial>>& aMaterials);
    static Red::Handle<Red::CMaterialInstance> CloneMaterialInstance(
        const Red::Handle<Red::CMaterialInstance>& aSourceInstance, const Core::SharedPtr<MeshState>& aMeshState,
        Red::CName aMaterialName, Red::JobQueue& aJobQueue, bool aAppendExtraContextPatams = false);
    static void ExpandMaterialInstanceParams(Red::Handle<Red::CMaterialInstance>& aMaterialInstance,
                                             const Core::SharedPtr<MeshState>& aMeshState, Red::CName aMaterialName,
                                             Red::JobQueue& aJobQueue);
    template<typename T>
    static bool ExpandResourceReference(Red::ResourceReference<T>& aRef, const Core::SharedPtr<MeshState>& aState,
                                        Red::CName aMaterialName);
    static Red::ResourcePath ExpandResourcePath(Red::ResourcePath aPath, const Core::SharedPtr<MeshState>& aState,
                                                Red::CName aMaterialName);

    template<typename T>
    static void EnsureResourceLoaded(Red::ResourceReference<T>& aRef);
    template<typename T>
    static void EnsureResourceLoaded(Red::SharedPtr<Red::ResourceToken<T>>& aToken);

    static Core::SharedPtr<MeshState> AcquireMeshState(Red::CMesh* aMesh);

    inline static Core::Map<Red::ResourcePath, Core::SharedPtr<MeshState>> s_states;
    inline static Red::Handle<Red::CMesh> s_dummyMesh;
    inline static Red::Handle<Red::meshMeshAppearance> s_dummyAppearance;
    inline static Red::Handle<Red::CMaterialInstance> s_dummyMaterial;
    inline static Red::SharedSpinLock s_stateLock;
};
}
