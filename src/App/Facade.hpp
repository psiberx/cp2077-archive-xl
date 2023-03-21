#pragma once

#include "App/Project.hpp"

namespace App
{
class Facade : public Red::IScriptable
{
public:
    static void Reload();
    static bool Require(Red::CString& aVersion);
    static Red::CString GetVersion();

    RTTI_IMPL_TYPEINFO(Facade);
};
}

RTTI_DEFINE_CLASS(App::Facade, App::Project::Name, {
    RTTI_ABSTRACT();
    RTTI_METHOD(Reload);
    RTTI_METHOD(Require);
    RTTI_METHOD(GetVersion, "Version");
})
