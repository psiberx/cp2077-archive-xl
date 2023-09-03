#include "Wrapper.hpp"
#include "Red/Mesh.hpp"

namespace
{
Red::ClassLocator<Red::ent::MeshComponent> s_MeshComponentRttiType;
Red::ClassLocator<Red::ent::SkinnedMeshComponent> s_SkinnedMeshComponentRttiType;
Red::ClassLocator<Red::ent::SkinnedClothComponent> s_SkinnedClothComponentRttiType;
Red::ClassLocator<Red::ent::MorphTargetSkinnedMeshComponent> s_MorphTargetSkinnedMeshComponentRttiType;

template<class T>
inline Red::ResourcePath GetComponentMeshPath(T* aComponent)
{
    return aComponent->mesh.path;
}

template<class T>
inline bool SetComponentMeshPath(T* aComponent, Red::ResourcePath aPath)
{
    aComponent->mesh = aPath;
    return true;
}

template<class T>
inline Red::ResourcePath GetMorphComponentMeshPath(T* aComponent)
{
    return aComponent->morphResource.path;
}

template<class T>
inline bool SetMorphComponentMeshPath(T* aComponent, Red::ResourcePath aPath)
{
    aComponent->morphResource = aPath;
    return true;
}

template<class T>
inline Red::CName GetComponentAppearanceName(T* aComponent)
{
    return aComponent->meshAppearance;
}

template<class T>
inline bool SetComponentAppearance(T* aComponent, Red::CName aAppearance)
{
    aComponent->meshAppearance = aAppearance;
    return true;
}

template<class T>
inline uint64_t GetComponentChunkMask(T* aComponent)
{
    return aComponent->chunkMask;
}

template<class T>
inline bool SetComponentChunkMask(T* aComponent, uint64_t aChunkMask)
{
    aComponent->chunkMask = aChunkMask;
    return true;
}

template<class T>
inline Red::ResourceReference<Red::CMesh> GetComponentMesh(T* aComponent)
{
    return aComponent->mesh.Resolve();
}

template<class T>
inline Red::ResourceReference<Red::CMesh> GetMorphComponentMesh(T* aComponent)
{
    Red::ResourceReference<Red::MorphTargetMesh> morphRef = aComponent->morphResource.Resolve();

    if (morphRef.LoadAsync() && morphRef.IsLoaded())
    {
        return morphRef.Get()->baseMesh;
    }

    return {};
}
}

App::ComponentWrapper::ComponentWrapper(Red::Handle<Red::IComponent>& aComponent)
    : m_component(aComponent)
    , m_type(ComponentType::Unknown)
    , m_uniqueId(0)
{
    const auto rttiType = m_component->GetType();

    if (rttiType->IsA(s_MeshComponentRttiType))
    {
        m_type = ComponentType::MeshComponent;
    }
    else if (rttiType->IsA(s_SkinnedMeshComponentRttiType))
    {
        m_type = ComponentType::SkinnedMeshComponent;
    }
    // else if (rttiType->IsA(s_SkinnedClothComponentRttiType))
    // {
    //     m_type = ComponentType::SkinnedClothComponent;
    // }
    else if (rttiType->IsA(s_MorphTargetSkinnedMeshComponentRttiType))
    {
        m_type = ComponentType::MorphTargetSkinnedMeshComponent;
    }
}

bool App::ComponentWrapper::IsMeshComponent() const
{
    return m_type != ComponentType::Unknown &&  m_type != ComponentType::SkinnedClothComponent;
}

uint64_t App::ComponentWrapper::GetUniqueId()
{
    if (!m_uniqueId)
    {
        // Component Name + Appearance Name + Appearance Resource Path
        m_uniqueId = m_component->name;
        m_uniqueId = Red::FNV1a64(m_component->unk48, sizeof(uint64_t), m_uniqueId);
        m_uniqueId = Red::FNV1a64(m_component->unk68, sizeof(uint64_t), m_uniqueId);
    }

    return m_uniqueId;
}

bool App::ComponentWrapper::IsEnabled() const
{
    return m_component->isEnabled;
}

bool App::ComponentWrapper::SetEnabled(bool isEnabled) const
{
    m_component->isEnabled = isEnabled;
    return true;
}

Red::ResourcePath App::ComponentWrapper::GetResource() const
{
    switch (m_type)
    {
    case ComponentType::MeshComponent:
        return GetComponentMeshPath(m_component.GetPtr<Red::ent::MeshComponent>());
    case ComponentType::SkinnedMeshComponent:
        return GetComponentMeshPath(m_component.GetPtr<Red::ent::SkinnedMeshComponent>());
    case ComponentType::SkinnedClothComponent:
        break;
    case ComponentType::MorphTargetSkinnedMeshComponent:
        return GetMorphComponentMeshPath(m_component.GetPtr<Red::ent::MorphTargetSkinnedMeshComponent>());
    case ComponentType::Unknown:
        break;
    }

    return {};
}

bool App::ComponentWrapper::SetResource(Red::ResourcePath aPath) const
{
    switch (m_type)
    {
    case ComponentType::MeshComponent:
        return SetComponentMeshPath(m_component.GetPtr<Red::ent::MeshComponent>(), aPath);
    case ComponentType::SkinnedMeshComponent:
        return SetComponentMeshPath(m_component.GetPtr<Red::ent::SkinnedMeshComponent>(), aPath);
    case ComponentType::SkinnedClothComponent:
        break;
    case ComponentType::MorphTargetSkinnedMeshComponent:
        return SetMorphComponentMeshPath(m_component.GetPtr<Red::ent::MorphTargetSkinnedMeshComponent>(), aPath);
    case ComponentType::Unknown:
        break;
    }

    return false;
}

Red::SharedPtr<Red::ResourceToken<Red::CMesh>> App::ComponentWrapper::LoadResource(bool aWait) const
{
    Red::ResourceReference<Red::CMesh> meshRef;

    switch (m_type)
    {
    case ComponentType::MeshComponent:
        meshRef = GetComponentMesh(m_component.GetPtr<Red::ent::MeshComponent>());
        break;
    case ComponentType::SkinnedMeshComponent:
        meshRef = GetComponentMesh(m_component.GetPtr<Red::ent::SkinnedMeshComponent>());
        break;
        break;
    case ComponentType::MorphTargetSkinnedMeshComponent:
        meshRef = GetMorphComponentMesh(m_component.GetPtr<Red::ent::MorphTargetSkinnedMeshComponent>());
        break;
    case ComponentType::SkinnedClothComponent:
    case ComponentType::Unknown:
        break;
    }

    if (meshRef.path)
    {
        meshRef.LoadAsync();

        if (aWait)
        {
            Red::WaitForResource(meshRef, std::chrono::milliseconds(1000));
        }
    }

    return meshRef.token;
}

Red::CName App::ComponentWrapper::GetAppearanceName() const
{
    switch (m_type)
    {
    case ComponentType::MeshComponent:
        return GetComponentAppearanceName(m_component.GetPtr<Red::ent::MeshComponent>());
    case ComponentType::SkinnedMeshComponent:
        return GetComponentAppearanceName(m_component.GetPtr<Red::ent::SkinnedMeshComponent>());
    case ComponentType::SkinnedClothComponent:
        return GetComponentAppearanceName(m_component.GetPtr<Red::ent::SkinnedClothComponent>());
    case ComponentType::MorphTargetSkinnedMeshComponent:
        return GetComponentAppearanceName(m_component.GetPtr<Red::ent::MorphTargetSkinnedMeshComponent>());
    case ComponentType::Unknown:
        break;
    }

    return {};
}

bool App::ComponentWrapper::SetAppearanceName(Red::CName aAppearance) const
{
    switch (m_type)
    {
    case ComponentType::MeshComponent:
        return SetComponentAppearance(m_component.GetPtr<Red::ent::MeshComponent>(), aAppearance);
    case ComponentType::SkinnedMeshComponent:
        return SetComponentAppearance(m_component.GetPtr<Red::ent::SkinnedMeshComponent>(), aAppearance);
    case ComponentType::SkinnedClothComponent:
        return SetComponentAppearance(m_component.GetPtr<Red::ent::SkinnedClothComponent>(), aAppearance);
    case ComponentType::MorphTargetSkinnedMeshComponent:
        return SetComponentAppearance(m_component.GetPtr<Red::ent::MorphTargetSkinnedMeshComponent>(), aAppearance);
    case ComponentType::Unknown:
        break;
    }

    return false;
}

bool App::ComponentWrapper::LoadAppearance() const
{
    Red::ResourceReference<Red::CMesh> meshRef;
    Red::CName meshAppName;

    switch (m_type)
    {
    case ComponentType::MeshComponent:
        meshRef = GetComponentMesh(m_component.GetPtr<Red::ent::MeshComponent>());
        meshAppName = GetComponentAppearanceName(m_component.GetPtr<Red::ent::MeshComponent>());
        break;
    case ComponentType::SkinnedMeshComponent:
        meshRef = GetComponentMesh(m_component.GetPtr<Red::ent::SkinnedMeshComponent>());
        meshAppName = GetComponentAppearanceName(m_component.GetPtr<Red::ent::SkinnedMeshComponent>());
        break;
    case ComponentType::SkinnedClothComponent:
        // meshRef = GetClothComponentMesh(m_component.GetPtr<Red::ent::SkinnedClothComponent>());
        // meshAppName = GetComponentAppearance(m_component.GetPtr<Red::ent::SkinnedClothComponent>());
        break;
    case ComponentType::MorphTargetSkinnedMeshComponent:
        meshRef = GetMorphComponentMesh(m_component.GetPtr<Red::ent::MorphTargetSkinnedMeshComponent>());
        meshAppName = GetComponentAppearanceName(m_component.GetPtr<Red::ent::MorphTargetSkinnedMeshComponent>());
        break;
    case ComponentType::Unknown:
        break;
    }

    // todo: forced option

    if (meshRef.LoadAsync() && meshRef.IsLoaded())
    {
        auto meshApp = Raw::CMesh::GetAppearance(*meshRef.Get().GetPtr(), meshAppName);
        if (meshApp && meshApp->name == meshAppName)
        {
            Raw::MeshAppearance::LoadMaterialSetupAsync(*meshApp.GetPtr(), meshApp, 0);
        }
    }

    return true;
}

uint64_t App::ComponentWrapper::GetChunkMask() const
{
    switch (m_type)
    {
    case ComponentType::MeshComponent:
        return GetComponentChunkMask(m_component.GetPtr<Red::ent::MeshComponent>());
    case ComponentType::SkinnedMeshComponent:
        return GetComponentChunkMask(m_component.GetPtr<Red::ent::SkinnedMeshComponent>());
    case ComponentType::SkinnedClothComponent:
        return GetComponentChunkMask(m_component.GetPtr<Red::ent::SkinnedClothComponent>());
    case ComponentType::MorphTargetSkinnedMeshComponent:
        return GetComponentChunkMask(m_component.GetPtr<Red::ent::MorphTargetSkinnedMeshComponent>());
    case ComponentType::Unknown:
        break;
    }

    return 0;
}

bool App::ComponentWrapper::SetChunkMask(uint64_t aChunkMask) const
{
    switch (m_type)
    {
    case ComponentType::MeshComponent:
        return SetComponentChunkMask(m_component.GetPtr<Red::ent::MeshComponent>(), aChunkMask);
    case ComponentType::SkinnedMeshComponent:
        return SetComponentChunkMask(m_component.GetPtr<Red::ent::SkinnedMeshComponent>(), aChunkMask);
    case ComponentType::SkinnedClothComponent:
        return SetComponentChunkMask(m_component.GetPtr<Red::ent::SkinnedClothComponent>(), aChunkMask);
    case ComponentType::MorphTargetSkinnedMeshComponent:
        return SetComponentChunkMask(m_component.GetPtr<Red::ent::MorphTargetSkinnedMeshComponent>(), aChunkMask);
    case ComponentType::Unknown:
        break;
    }

    return false;
}
