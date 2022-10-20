#include "RttiProvider.hpp"
#include "Red/Rtti/Registrar.hpp"

void Red::RttiProvider::OnBootstrap()
{
    Red::Rtti::Registrar::RegisterPending();
}
