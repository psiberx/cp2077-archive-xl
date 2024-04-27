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
    static void* OnLoadMaterials(Red::CMesh* aMesh, Red::MeshMaterialsToken& aToken,
                                 const Red::DynArray<Red::CName>& aMaterialNames, uint8_t a4);

    template<typename T>
    static bool ProcessResourceReference(Red::ResourceReference<T>& aInstance, Red::CName aMaterialName);
    static bool ProcessMeshResource(Red::CMesh* aMesh, const Red::DynArray<Red::CName>& aMaterialNames,
                                    Core::Vector<Red::JobHandle>& aLoadingJobs);

    inline static std::shared_mutex s_materiaLock;
};
}
