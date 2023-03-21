#include "TypeInfoProvider.hpp"
#include "Red/Rtti/Registrar.hpp"

void Red::TypeInfoProvider::OnBootstrap()
{
    Red::RTTIRegistrar::RegisterPending();
}
