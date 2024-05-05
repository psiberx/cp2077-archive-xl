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

    static bool IsSpecialMaterial(Red::CName aMaterialName);

private:
    struct MeshState
    {
        MeshState(Red::CMesh* aMesh);

        void MarkStatic();
        [[nodiscard]] bool IsDynamic() const;
        void FillAttributes(Red::CMesh* aMesh);

        std::shared_mutex mutex;
        volatile bool dynamic;
        DynamicAttributeList attrs;
        Red::WeakHandle<Red::CMesh> mesh;
    };

    static void* OnLoadMaterials(Red::CMesh* aMesh, Red::MeshMaterialsToken& aToken,
                                 const Red::DynArray<Red::CName>& aMaterialNames, uint8_t a4);

    static bool ProcessMeshResource(Red::CMesh* aMesh, const Red::DynArray<Red::CName>& aMaterialNames,
                                    Core::Vector<Red::JobHandle>& aLoadingJobs);
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
    inline static std::shared_mutex s_stateLock;
};
}
