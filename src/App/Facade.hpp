#pragma once

#include "App/Project.hpp"

namespace App
{
class Facade : public Red::IScriptable
{
public:
    static bool RegisterDir(Red::CString& aPath);
    static bool RegisterArchive(Red::CString& aPath);
    static Red::CName GetBodyType(const Red::WeakHandle<Red::GameObject>& aPuppet);
    static void EnableGarmentOffsets();
    static void DisableGarmentOffsets();
    static void EnablePhotoModeAlternativeControls();
    static void DisablePhotoModeAlternativeControls();
    static void Reload();
    static bool Require(Red::CString& aVersion);
    static Red::CString GetVersion();

    RTTI_IMPL_TYPEINFO(Facade);
};
}

RTTI_DEFINE_CLASS(App::Facade, App::Project::Name, {
    RTTI_ABSTRACT();
    RTTI_METHOD(RegisterDir);
    RTTI_METHOD(RegisterArchive);
    RTTI_METHOD(GetBodyType);
    RTTI_METHOD(EnableGarmentOffsets);
    RTTI_METHOD(DisableGarmentOffsets);
    RTTI_METHOD(EnablePhotoModeAlternativeControls);
    RTTI_METHOD(DisablePhotoModeAlternativeControls);
    RTTI_METHOD(Reload);
    RTTI_METHOD(Require);
    RTTI_METHOD(GetVersion, "Version");
})
