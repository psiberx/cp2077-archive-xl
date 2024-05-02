#include "Wrapper.hpp"
#include "Red/Mesh.hpp"

namespace
{
Red::ClassLocator<Red::ent::MeshComponent> s_MeshComponentRttiType;
Red::ClassLocator<Red::ent::SkinnedMeshComponent> s_SkinnedMeshComponentRttiType;
Red::ClassLocator<Red::ent::GarmentSkinnedMeshComponent> s_garmentSkinnedMeshComponentRttiType;
Red::ClassLocator<Red::ent::MorphTargetSkinnedMeshComponent> s_MorphTargetSkinnedMeshComponentRttiType;

template<class T>
inline Red::ResourcePath GetComponentResourcePath(T* aComponent)
{
    return aComponent->mesh.path;
}

template<>
inline Red::ResourcePath GetComponentResourcePath(Red::ent::MorphTargetSkinnedMeshComponent* aComponent)
{
    return aComponent->morphResource.path;
}

template<class T>
inline bool SetComponentResourcePath(T* aComponent, Red::ResourcePath aPath)
{
    aComponent->mesh = aPath;
    return true;
}

template<>
inline bool SetComponentResourcePath(Red::ent::MorphTargetSkinnedMeshComponent* aComponent, Red::ResourcePath aPath)
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
inline Red::ResourceReference<Red::CMesh> GetComponentMeshReference(T* aComponent)
{
    return aComponent->mesh.Resolve();
}

template<>
inline Red::ResourceReference<Red::CMesh> GetComponentMeshReference(Red::ent::MorphTargetSkinnedMeshComponent* aComponent)
{
    Red::ResourceReference<Red::MorphTargetMesh> morphRef = aComponent->morphResource.Resolve();

    if (morphRef.LoadAsync() && morphRef.IsLoaded())
    {
        return morphRef.token->resource->baseMesh;
    }

    return {};
}
}

App::ComponentWrapper::ComponentWrapper(Red::IComponent* aComponent)
    : m_component(aComponent)
    , m_componentType(ComponentType::Unsupported)
    , m_uniqueId(0)
{
    const auto rttiType = m_component->GetType();

    if (rttiType->IsA(s_MeshComponentRttiType))
    {
        m_componentType = ComponentType::MeshComponent;
    }
    else if (rttiType->IsA(s_MorphTargetSkinnedMeshComponentRttiType))
    {
        m_componentType = ComponentType::MorphTargetSkinnedMeshComponent;
    }
    else if (rttiType->IsA(s_garmentSkinnedMeshComponentRttiType))
    {
        m_componentType = ComponentType::GarmentSkinnedMeshComponent;
    }
    else if (rttiType->IsA(s_SkinnedMeshComponentRttiType))
    {
        m_componentType = ComponentType::SkinnedMeshComponent;
    }
}

App::ComponentWrapper::ComponentWrapper(const Red::Handle<Red::IComponent>& aComponent)
    : ComponentWrapper(aComponent.instance)
{
}

bool App::ComponentWrapper::IsMeshComponent() const
{
    return m_componentType != ComponentType::Unsupported;
}

bool App::ComponentWrapper::IsGarmentComponent() const
{
    return m_componentType == ComponentType::GarmentSkinnedMeshComponent ||
           m_componentType == ComponentType::MorphTargetSkinnedMeshComponent;
}

uint64_t App::ComponentWrapper::GetUniqueId()
{
    if (!m_uniqueId)
    {
        // Component Name + Appearance Name + Appearance Resource Path
        m_uniqueId = m_component->name;
        m_uniqueId = Red::FNV1a64(reinterpret_cast<const uint8_t*>(&m_component->appearanceName), sizeof(uint64_t), m_uniqueId);
        m_uniqueId = Red::FNV1a64(reinterpret_cast<const uint8_t*>(&m_component->appearancePath), sizeof(uint64_t), m_uniqueId);
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

Red::ResourcePath App::ComponentWrapper::GetResourcePath() const
{
    switch (m_componentType)
    {
    case ComponentType::MeshComponent:
        return GetComponentResourcePath(reinterpret_cast<Red::ent::MeshComponent*>(m_component));
    case ComponentType::SkinnedMeshComponent:
    case ComponentType::GarmentSkinnedMeshComponent:
        return GetComponentResourcePath(reinterpret_cast<Red::ent::SkinnedMeshComponent*>(m_component));
    case ComponentType::MorphTargetSkinnedMeshComponent:
        return GetComponentResourcePath(reinterpret_cast<Red::ent::MorphTargetSkinnedMeshComponent*>(m_component));
    case ComponentType::Unsupported:
        break;
    }

    return {};
}

bool App::ComponentWrapper::SetResourcePath(Red::ResourcePath aPath) const
{
    switch (m_componentType)
    {
    case ComponentType::MeshComponent:
        return SetComponentResourcePath(reinterpret_cast<Red::ent::MeshComponent*>(m_component), aPath);
    case ComponentType::SkinnedMeshComponent:
    case ComponentType::GarmentSkinnedMeshComponent:
        return SetComponentResourcePath(reinterpret_cast<Red::ent::SkinnedMeshComponent*>(m_component), aPath);
    case ComponentType::MorphTargetSkinnedMeshComponent:
        return SetComponentResourcePath(reinterpret_cast<Red::ent::MorphTargetSkinnedMeshComponent*>(m_component), aPath);
    case ComponentType::Unsupported:
        break;
    }

    return false;
}

bool App::ComponentWrapper::LoadResource(bool aWait) const
{
    if (!IsMeshComponent())
        return false;

    Red::JobQueue jobQueue;
    Raw::MeshComponent::LoadResource(m_component, jobQueue);

    if (aWait)
    {
        Red::WaitForQueue(jobQueue, std::chrono::milliseconds(1000));
    }

    return true;
}

Red::SharedPtr<Red::ResourceToken<Red::CMesh>> App::ComponentWrapper::LoadResourceToken(bool aWait) const
{
    Red::ResourceReference<Red::CMesh> meshRef;

    switch (m_componentType)
    {
    case ComponentType::MeshComponent:
        meshRef = GetComponentMeshReference(reinterpret_cast<Red::ent::MeshComponent*>(m_component));
        break;
    case ComponentType::SkinnedMeshComponent:
    case ComponentType::GarmentSkinnedMeshComponent:
        meshRef = GetComponentMeshReference(reinterpret_cast<Red::ent::SkinnedMeshComponent*>(m_component));
        break;
    case ComponentType::MorphTargetSkinnedMeshComponent:
        meshRef = GetComponentMeshReference(reinterpret_cast<Red::ent::MorphTargetSkinnedMeshComponent*>(m_component));
        break;
    case ComponentType::Unsupported:
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
    switch (m_componentType)
    {
    case ComponentType::MeshComponent:
        return GetComponentAppearanceName(reinterpret_cast<Red::ent::MeshComponent*>(m_component));
    case ComponentType::SkinnedMeshComponent:
    case ComponentType::GarmentSkinnedMeshComponent:
        return GetComponentAppearanceName(reinterpret_cast<Red::ent::SkinnedMeshComponent*>(m_component));
    case ComponentType::MorphTargetSkinnedMeshComponent:
        return GetComponentAppearanceName(reinterpret_cast<Red::ent::MorphTargetSkinnedMeshComponent*>(m_component));
    case ComponentType::Unsupported:
        break;
    }

    return {};
}

bool App::ComponentWrapper::SetAppearanceName(Red::CName aAppearance) const
{
    switch (m_componentType)
    {
    case ComponentType::MeshComponent:
        return SetComponentAppearance(reinterpret_cast<Red::ent::MeshComponent*>(m_component), aAppearance);
    case ComponentType::SkinnedMeshComponent:
    case ComponentType::GarmentSkinnedMeshComponent:
        return SetComponentAppearance(reinterpret_cast<Red::ent::SkinnedMeshComponent*>(m_component), aAppearance);
    case ComponentType::MorphTargetSkinnedMeshComponent:
        return SetComponentAppearance(reinterpret_cast<Red::ent::MorphTargetSkinnedMeshComponent*>(m_component), aAppearance);
    case ComponentType::Unsupported:
        break;
    }

    return false;
}

bool App::ComponentWrapper::LoadAppearance() const
{
    Red::ResourceReference<Red::CMesh> meshRef;
    Red::CName meshAppName;

    switch (m_componentType)
    {
    case ComponentType::MeshComponent:
        meshRef = GetComponentMeshReference(reinterpret_cast<Red::ent::MeshComponent*>(m_component));
        meshAppName = GetComponentAppearanceName(reinterpret_cast<Red::ent::MeshComponent*>(m_component));
        break;
    case ComponentType::SkinnedMeshComponent:
    case ComponentType::GarmentSkinnedMeshComponent:
        meshRef = GetComponentMeshReference(reinterpret_cast<Red::ent::SkinnedMeshComponent*>(m_component));
        meshAppName = GetComponentAppearanceName(reinterpret_cast<Red::ent::SkinnedMeshComponent*>(m_component));
        break;
    case ComponentType::MorphTargetSkinnedMeshComponent:
        meshRef = GetComponentMeshReference(reinterpret_cast<Red::ent::MorphTargetSkinnedMeshComponent*>(m_component));
        meshAppName = GetComponentAppearanceName(reinterpret_cast<Red::ent::MorphTargetSkinnedMeshComponent*>(m_component));
        break;
    case ComponentType::Unsupported:
        break;
    }

    // todo: forced option

    if (meshRef.LoadAsync() && meshRef.IsLoaded())
    {
        auto meshApp = Raw::CMesh::GetAppearance(meshRef.token->resource, meshAppName);
        if (meshApp && meshApp->name == meshAppName)
        {
            Raw::MeshAppearance::LoadMaterialSetupAsync(*meshApp.GetPtr(), meshApp, 0);
        }
    }

    return true;
}

uint64_t App::ComponentWrapper::GetChunkMask() const
{
    switch (m_componentType)
    {
    case ComponentType::MeshComponent:
        return GetComponentChunkMask(reinterpret_cast<Red::ent::MeshComponent*>(m_component));
    case ComponentType::SkinnedMeshComponent:
    case ComponentType::GarmentSkinnedMeshComponent:
        return GetComponentChunkMask(reinterpret_cast<Red::ent::SkinnedMeshComponent*>(m_component));
    case ComponentType::MorphTargetSkinnedMeshComponent:
        return GetComponentChunkMask(reinterpret_cast<Red::ent::MorphTargetSkinnedMeshComponent*>(m_component));
    case ComponentType::Unsupported:
        break;
    }

    return 0;
}

bool App::ComponentWrapper::SetChunkMask(uint64_t aChunkMask) const
{
    switch (m_componentType)
    {
    case ComponentType::MeshComponent:
        return SetComponentChunkMask(reinterpret_cast<Red::ent::MeshComponent*>(m_component), aChunkMask);
    case ComponentType::SkinnedMeshComponent:
    case ComponentType::GarmentSkinnedMeshComponent:
        return SetComponentChunkMask(reinterpret_cast<Red::ent::SkinnedMeshComponent*>(m_component), aChunkMask);
    case ComponentType::MorphTargetSkinnedMeshComponent:
        return SetComponentChunkMask(reinterpret_cast<Red::ent::MorphTargetSkinnedMeshComponent*>(m_component), aChunkMask);
    case ComponentType::Unsupported:
        break;
    }

    return false;
}
