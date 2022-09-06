#include "RTTIProvider.hpp"
#include "Engine/Scripting/RTTIRegistrar.hpp"

void Engine::RTTIProvider::OnBootstrap()
{
    Engine::RTTIRegistrar::RegisterPending();
}
