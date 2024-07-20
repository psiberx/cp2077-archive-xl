#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "App/Extensions/GarmentOverride/Dynamic.hpp"
#include "Red/Mesh.hpp"

namespace App
{
class MeshTemplateModule : public Module
{
public:
    bool Load() override;
    bool Unload() override;
    std::string_view GetName() override;

    static bool IsContextualMesh(Red::CMesh* aMesh);
    static bool IsSpecialMaterial(Red::CName aMaterialName);
    static void PrefetchMeshState(Red::CMesh* aMesh, const Core::Map<Red::CName, std::string>& aContext);

private:
    struct MeshState
    {
        MeshState(Red::CMesh* aMesh);

        void MarkStatic();
        [[nodiscard]] bool IsStatic() const;

        void PrefetchContext(Red::CMesh* aMesh);
        void FillContext(const Core::Map<Red::CName, std::string>& aContext);
        const DynamicAttributeList& GetContext();

        void FillMaterials(Red::CMesh* aMesh);
        void RegisterMaterialEntry(Red::CName aMaterialName, int32_t aEntryIndex);
        [[nodiscard]] int32_t GetTemplateEntryIndex(Red::CName aMaterialName);
        [[nodiscard]] int32_t GetMaterialEntryIndex(Red::CName aMaterialName);
        [[nodiscard]] bool HasMaterialEntry(Red::CName aMaterialName) const;

        Red::CName RegisterSource(Red::CMesh* aSourceMesh);
        [[nodiscard]] Red::CMesh* ResolveSource(Red::CName aSourceName);

        volatile bool dynamic;
        std::shared_mutex meshMutex;
        std::shared_mutex sourceMutex;
        Red::SharedPtr<Red::ResourceToken<Red::IMaterial>> contextToken;
        DynamicAttributeList context;
        Core::Map<Red::CName, int32_t> templates;
        Core::Map<Red::CName, int32_t> materials;
        Core::Map<Red::CName, Red::CMesh*> sources;
    };

    static void OnFindAppearance(Red::Handle<Red::mesh::MeshAppearance>& aOut, Red::CMesh* aMesh, Red::CName aName);
    static void OnLoadMaterials(Red::CMesh* aMesh, Red::MeshMaterialsToken& aToken,
                                const Red::DynArray<Red::CName>& aMaterialNames, uint8_t a4);
    static void OnAddStubAppearance(Red::CMesh* aMesh);
    static void OnPreloadAppearances(bool& aResult, Red::CMesh* aMesh);

    static bool ProcessMeshResource(Red::CMesh* aMesh, const Red::DynArray<Red::CName>& aMaterialNames,
                                    Red::DynArray<Red::Handle<Red::IMaterial>>& aFinalMaterials);
    static bool ContainsUnresolvedMaterials(const Red::DynArray<Red::Handle<Red::IMaterial>>& aMaterials);
    static Red::Handle<Red::CMaterialInstance> CloneMaterialInstance(
        const Red::Handle<Red::CMaterialInstance>& aSourceInstance, MeshState* aState, Red::CName aMaterialName,
        Core::Vector<Red::JobHandle>& aLoadingJobs);
    static void ExpandMaterialInstanceParams(Red::Handle<Red::CMaterialInstance>& aMaterialInstance, MeshState* aState,
                                              Red::CName aMaterialName, Core::Vector<Red::JobHandle>& aLoadingJobs);
    template<typename T>
    static bool ExpandResourceReference(Red::ResourceReference<T>& aRef, MeshState* aState, Red::CName aMaterialName);
    static Red::ResourcePath ExpandResourcePath(Red::ResourcePath aPath, MeshState* aState, Red::CName aMaterialName);

    template<typename T>
    static void EnsureResourceLoaded(Red::ResourceReference<T>& aRef);
    template<typename T>
    static void EnsureResourceLoaded(Red::SharedPtr<Red::ResourceToken<T>>& aToken);

    static MeshState* AcquireMeshState(Red::CMesh* aMesh);

    inline static Core::Map<Red::ResourcePath, Core::UniquePtr<MeshState>> s_states;
    inline static Red::Handle<Red::CMaterialInstance> s_dummyMaterial;
    inline static std::shared_mutex s_stateLock;
};
}
