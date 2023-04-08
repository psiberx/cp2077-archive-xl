#include "RedLibProvider.hpp"
#include "Red/TypeInfo/Registrar.hpp"

void Support::RedLibProvider::OnBootstrap()
{
    Red::TypeInfoRegistrar::RegisterDiscovered();
}
