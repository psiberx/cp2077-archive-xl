#include "Extension.hpp"
#include "Red/GameEngine.hpp"

namespace
{
constexpr auto ExtensionName = "InkSpawner";

constexpr auto ControllerSeparator = ':';

Red::ClassLocator<Red::inkIWidgetController> s_gameControllerType;
Red::ClassLocator<Red::inkWidgetLogicController> s_logicControllerType;
}

std::string_view App::InkSpawnerExtension::GetName()
{
    return ExtensionName;
}

bool App::InkSpawnerExtension::Load()
{
    HookOnceAfter<Raw::CBaseEngine::InitEngine>(+[]() {
        if (Red::GetType<"Codeware">())
        {
            bool isNewCodeware{false};
            Red::CString newCodewareVersion{"1.14.0"};
            Red::CallStatic("Codeware", "Require", isNewCodeware, newCodewareVersion);

            if (isNewCodeware)
                return;
        }

        Hook<Raw::InkWidgetLibrary::SpawnFromLocal>(&OnSpawnLocal).OrThrow();
        Hook<Raw::InkWidgetLibrary::SpawnFromExternal>(&OnSpawnExternal).OrThrow();
        Hook<Raw::InkWidgetLibrary::AsyncSpawnFromLocal>(&OnAsyncSpawnLocal).OrThrow();
        Hook<Raw::InkWidgetLibrary::AsyncSpawnFromExternal>(&OnAsyncSpawnExternal).OrThrow();
        Hook<Raw::InkWidgetLibrary::FindExternalLibrary>(&OnFindExternalLibrary).OrThrow();
        HookBefore<Raw::InkSpawner::FinishAsyncSpawn>(&OnFinishAsyncSpawn).OrThrow();
    });

    return true;
}

bool App::InkSpawnerExtension::Unload()
{
    Unhook<Raw::InkWidgetLibrary::SpawnFromLocal>();
    Unhook<Raw::InkWidgetLibrary::SpawnFromExternal>();
    Unhook<Raw::InkWidgetLibrary::AsyncSpawnFromLocal>();
    Unhook<Raw::InkWidgetLibrary::AsyncSpawnFromExternal>();
    Unhook<Raw::InkWidgetLibrary::FindExternalLibrary>();
    Unhook<Raw::InkSpawner::FinishAsyncSpawn>();

    return true;
}

void* App::InkSpawnerExtension::OnSpawnLocal(Red::inkWidgetLibraryResource* aLibrary,
                                               Red::Handle<Red::inkWidgetLibraryItemInstance>& aOut,
                                               Red::CName aItemName)
{
    auto result = Raw::InkWidgetLibrary::SpawnFromLocal(aLibrary, aOut, aItemName);

    if (!aOut)
    {
        auto* itemNameStr = aItemName.ToString();
        auto* controllerSep = strchr(itemNameStr, ControllerSeparator);

        if (controllerSep)
        {
            aItemName = Red::CName(itemNameStr, controllerSep - itemNameStr);
            Raw::InkWidgetLibrary::SpawnFromLocal(aLibrary, aOut, aItemName);

            if (aOut)
            {
                InjectController(aOut, controllerSep + 1);
            }
        }
    }

    return result;
}

void* App::InkSpawnerExtension::OnSpawnExternal(Red::inkWidgetLibraryResource* aLibrary,
                                                  Red::Handle<Red::inkWidgetLibraryItemInstance>& aOut,
                                                  Red::ResourcePath aExternalPath, Red::CName aItemName)
{
    if (aExternalPath && aLibrary->path != aExternalPath)
    {
        if (auto externalLibrary = ResolveDependency(aLibrary, aExternalPath))
        {
            return OnSpawnLocal(externalLibrary, aOut, aItemName);
        }
    }

    aOut.instance = nullptr;
    aOut.refCount = nullptr;

    return &aOut;
}

bool App::InkSpawnerExtension::OnAsyncSpawnLocal(Red::inkWidgetLibraryResource* aLibrary,
                                                   Red::InkSpawningInfo& aSpawningInfo, Red::CName aItemName,
                                                   bool aFlag)
{
    auto* itemNameStr = aItemName.ToString();
    auto* controllerSep = strchr(itemNameStr, ControllerSeparator);

    if (controllerSep)
    {
        aItemName = Red::CName(itemNameStr, controllerSep - itemNameStr);
    }

    return Raw::InkWidgetLibrary::AsyncSpawnFromLocal(aLibrary, aSpawningInfo, aItemName, aFlag);
}

bool App::InkSpawnerExtension::OnAsyncSpawnExternal(Red::inkWidgetLibraryResource* aLibrary,
                                                      Red::InkSpawningInfo& aSpawningInfo,
                                                      Red::ResourcePath aExternalPath,
                                                      Red::CName aItemName,
                                                      bool aFlag)
{
    if (aExternalPath && aLibrary->path != aExternalPath)
    {
        if (auto externalLibrary = ResolveDependency(aLibrary, aExternalPath))
        {
            return OnAsyncSpawnLocal(externalLibrary, aSpawningInfo, aItemName, aFlag);
        }
    }

    return false;
}

void* App::InkSpawnerExtension::OnFindExternalLibrary(Red::inkWidgetLibraryResource* aLibrary,
                                                        Red::Handle<Red::inkWidgetLibraryResource>& aOut,
                                                        Red::ResourcePath aExternalPath)
{
    std::shared_lock _(s_externalLibraryLock);

    return Raw::InkWidgetLibrary::FindExternalLibrary(aLibrary, aOut, aExternalPath);
}

void App::InkSpawnerExtension::OnFinishAsyncSpawn(Red::InkSpawningContext& aContext,
                                                    Red::Handle<Red::inkWidgetLibraryItemInstance>& aInstance)
{
    const char* itemNameStr = nullptr;
    const char* controllerSep = nullptr;

    {
        itemNameStr = aContext.request->itemName.ToString();
        controllerSep = strchr(itemNameStr, ControllerSeparator);

        if (controllerSep)
        {
            InjectController(aInstance, controllerSep + 1);
        }
    }
}

Red::Handle<Red::inkWidgetLibraryResource> App::InkSpawnerExtension::ResolveDependency(
    Red::inkWidgetLibraryResource* aLibrary, Red::ResourcePath aExternalPath)
{
    {
        std::shared_lock _(s_externalLibraryLock);

        for (const auto& externalLibrary : aLibrary->externalLibraries)
        {
            if (externalLibrary.path == aExternalPath)
            {
                return externalLibrary.token->resource;
            }
        }
    }

    auto token = Red::FetchResourceToken<Red::inkWidgetLibraryResource>(aExternalPath, std::chrono::milliseconds(1000));

    {
        std::unique_lock _(s_externalLibraryLock);

        aLibrary->externalLibraries.EmplaceBack(aExternalPath);
    }

    return token->resource;
}

void App::InkSpawnerExtension::InjectController(Red::Handle<Red::inkWidgetLibraryItemInstance>& aOut,
                                                  Red::CName aControllerName)
{
    auto* controllerType = Red::CRTTISystem::Get()->GetClass(aControllerName);

    if (controllerType)
    {
        if (controllerType->IsA(s_gameControllerType))
        {
            auto* controllerInstance = reinterpret_cast<Red::inkIWidgetController*>(controllerType->CreateInstance(true));
            Red::Handle<Red::inkIWidgetController> controllerHandle(controllerInstance);

            aOut->gameController.Swap(controllerHandle);

            if (controllerHandle.instance)
            {
                InheritProperties(controllerInstance, controllerHandle.instance);
            }
        }
        else if (controllerType->IsA(s_logicControllerType))
        {
            auto* controllerInstance = reinterpret_cast<Red::inkWidgetLogicController*>(controllerType->CreateInstance(true));
            Red::Handle<Red::inkWidgetLogicController> controllerHandle(controllerInstance);

            aOut->rootWidget->logicController.Swap(controllerHandle);

            if (controllerHandle.instance)
            {
                InheritProperties(controllerInstance, controllerHandle.instance);
            }
        }
    }
}

void App::InkSpawnerExtension::InheritProperties(Red::IScriptable* aTarget, Red::IScriptable* aSource)
{
    auto* sourceType = aSource->GetType();
    auto* targetType = aTarget->GetType();

    Red::DynArray<Red::CProperty*> sourceProps;
    sourceType->GetProperties(sourceProps);

    for (const auto& sourceProp : sourceProps)
    {
        const auto targetProp = targetType->GetProperty(sourceProp->name);

        if (targetProp && targetProp->type == sourceProp->type)
        {
            targetProp->SetValue(aTarget, sourceProp->GetValuePtr<void>(aSource));
        }
    }
}
