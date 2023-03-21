#pragma once

namespace Red
{
template<typename T>
constexpr uint64_t FNV1a64v(T aValue, uint64_t aSeed = 0xCBF29CE484222325)
{
    constexpr uint64_t prime = 0x100000001b3;

    uint64_t hash = aSeed;
    for (size_t i = 0; i != sizeof(T); ++i)
    {
        hash ^= aValue % 0xFF;
        hash *= prime;
        aValue /= 0xFF;
    }

    return hash;
}

consteval auto AutoScope(const std::source_location& aLocation = std::source_location::current()) {
    return FNV1a64v(aLocation.line(), FNV1a64(aLocation.file_name()));
}

template<typename T, auto ADefault = T()>
struct Optional
{
    inline operator T&()
    {
        return value;
    }

    T value{ADefault};
};

namespace Detail
{
template<typename T>
concept IsScripable = std::is_base_of_v<IScriptable, T>;

template<typename T>
concept IsType = std::is_base_of_v<CBaseRTTIType, T>;

template<typename T>
concept IsTypeOrVoid = IsType<T> or std::is_void_v<T>;

template<typename F>
struct PropertyPtr : public std::false_type {};

template<typename C, typename P>
struct PropertyPtr<P C::*> : public std::true_type
{
    using context_type = C;
    using value_type = P;
};

template<typename T>
concept IsPropertyPtr = PropertyPtr<T>::value;

template<typename F>
struct FunctionPtr : public std::false_type {};

template<typename R, typename... Args>
struct FunctionPtr<R (*)(Args...)> : public std::true_type
{
    using context_type = void;
    using return_type = R;

    using arguments_type = std::tuple<std::remove_const_t<std::remove_reference_t<Args>>...>;
    using qualified_arguments_type = std::tuple<Args...>;
    using extended_arguments_type = arguments_type;

    template<size_t I>
    using argument_type = typename std::tuple_element<I, arguments_type>::type;

    template<size_t I>
    using qualified_argument_type = typename std::tuple_element<I, qualified_arguments_type>::type;

    template<size_t I>
    using extended_argument_type = typename std::tuple_element<I, extended_arguments_type>::type;

    static constexpr size_t arity = sizeof...(Args);
};

template<typename C, typename R, typename... Args>
struct FunctionPtr<R (C::*)(Args...)> : FunctionPtr<R (*)(Args...)>
{
    using context_type = C;
    using extended_arguments_type = std::tuple<C*, std::remove_const_t<std::remove_reference_t<Args>>...>;
};

template<typename C, typename R, typename... Args>
struct FunctionPtr<R (C::*)(Args...) const> : FunctionPtr<R (C::*)(Args...)> {};

template<typename R, typename... Args>
struct FunctionPtr<R (Args...)> : FunctionPtr<R (*)(Args...)> {};

template<typename T>
concept IsFunctionPtr = FunctionPtr<T>::value;

template<typename T>
concept IsStaticFunctionPtr = FunctionPtr<T>::value && std::is_void_v<typename FunctionPtr<T>::context_type>;

template<typename T>
concept IsMemberFunctionPtr = FunctionPtr<T>::value && !std::is_void_v<typename FunctionPtr<T>::context_type>;

template<typename T>
struct Specialization : public std::false_type {};

template<template<typename> class G, typename A>
struct Specialization<G<A>> : public std::true_type
{
    using argument_type = A;
};

template<template<typename, typename...> class G, typename A, typename... Args>
struct Specialization<G<A, Args...>> : public std::true_type
{
    using argument_type = A;
};

template<template<typename, auto> class G, typename A, auto V>
struct Specialization<G<A, V>> : public std::true_type
{
    using argument_type = A;
    static constexpr auto argument_value = V;
};

template<typename T>
concept IsSpecialization = Specialization<T>::value;

// template<typename T, template<typename> class G>
// concept IsSameSpecialization = (IsSpecialization<T> && std::is_same_v<T, G<typename Specialization<T>::argument_type>>);

template<typename T>
concept IsOptional = Specialization<T>::value
    && std::is_same_v<T, Red::Optional<typename Specialization<T>::argument_type, Specialization<T>::argument_value>>;
}
}
