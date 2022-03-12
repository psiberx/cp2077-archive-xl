#include "RuntimeProvider.hpp"
#include "Core/Facades/Runtime.hpp"
#include "Core/Runtime/HostImage.hpp"
#include "Core/Runtime/ModuleImage.hpp"

Core::RuntimeProvider::RuntimeProvider(Core::RuntimeProvider::Options&& aOptions)
{
    Runtime::Initialize(
        HostImage(aOptions.exePathDepth),
        ModuleImage(aOptions.handle));
}
