#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "Red/Mesh.hpp"

namespace App
{
class MeshTemplateModule : public Module
{
public:
    bool Load() override;
    bool Unload() override;
    std::string_view GetName() override;

private:
    struct MeshState
    {
        MeshState(Red::CMesh* aMesh)
            : dynamic(true)
            , mesh(Red::ToHandle(aMesh))
        {
        }

        void MarkStatic()
        {
            dynamic = false;
            mesh.Reset();
        }

        [[nodiscard]] bool IsDynamic() const
        {
            return dynamic;
        }

        std::shared_mutex mutex;
        volatile bool dynamic;
        Red::WeakHandle<Red::CMesh> mesh;
    };

    static void* OnLoadMaterials(Red::CMesh* aMesh, Red::MeshMaterialsToken& aToken,
                                 const Red::DynArray<Red::CName>& aMaterialNames, uint8_t a4);

    template<typename T>
    static bool ProcessResourceReference(Red::ResourceReference<T>& aInstance, Red::CName aMaterialName);
    static bool ProcessMeshResource(Red::CMesh* aMesh, const Red::DynArray<Red::CName>& aMaterialNames,
                                    Core::Vector<Red::JobHandle>& aLoadingJobs);

    static MeshState* AcquireMeshState(Red::CMesh* aMesh);

    inline static Core::Map<Red::ResourcePath, Core::UniquePtr<MeshState>> s_states;
    inline static std::shared_mutex s_stateLock;
};
}
