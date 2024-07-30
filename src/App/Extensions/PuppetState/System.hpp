#pragma once

namespace App
{
struct PuppetStateSystem : Red::ScriptableSystem
{
    Red::CString GetBodyTypeSuffix(Red::ItemID& aItemID, const Red::WeakHandle<Red::GameObject>& aOwner,
                                   const Red::Handle<Red::ItemsFactoryAppearanceSuffixBase_Record>&);
    Red::CString GetArmsStateSuffix(Red::ItemID& aItemID, const Red::WeakHandle<Red::GameObject>& aOwner,
                                    const Red::Handle<Red::ItemsFactoryAppearanceSuffixBase_Record>&);
    Red::CString GetFeetStateSuffix(Red::ItemID& aItemID, const Red::WeakHandle<Red::GameObject>& aOwner,
                                    const Red::Handle<Red::ItemsFactoryAppearanceSuffixBase_Record>&);

    RTTI_IMPL_TYPEINFO(App::PuppetStateSystem);
    RTTI_FWD_CONSTRUCTOR();
};
}

RTTI_DEFINE_CLASS(App::PuppetStateSystem, {
    RTTI_SCRIPT_METHOD(GetBodyTypeSuffix);
    RTTI_SCRIPT_METHOD(GetArmsStateSuffix);
    RTTI_SCRIPT_METHOD(GetFeetStateSuffix);
})
