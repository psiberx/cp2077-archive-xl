#include "RuntimeProvider.hpp"
#include "Core/Facades/Runtime.hpp"

Core::RuntimeProvider::RuntimeProvider(HMODULE aHandle) noexcept
    : m_handle(aHandle)
    , m_basePathDepth(0)
{
}

void Core::RuntimeProvider::OnInitialize()
{
    Runtime::Initialize(HostImage(m_basePathDepth), ModuleImage(m_handle));
}
