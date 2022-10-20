#pragma once

#include "Detail.hpp"

#include <RED4ext/RTTISystem.hpp>

namespace Red::Rtti
{
/**
 * The registrar queues callbacks to register all at once on demand.
 * This serves two purposes:
 * - Auto discovery of used  descriptors.
 * - Postpone until  system is ready.
 */
class Registrar
{
public:
    using CallbackFunc = RED4ext::RTTIRegistrator::CallbackFunc;

    Registrar(CallbackFunc aRegFunc, CallbackFunc aBuildFunc);

    void Register();
    static void RegisterPending();

private:
    bool m_registered;
    CallbackFunc m_regFunc;
    CallbackFunc m_buildFunc;
};
}
