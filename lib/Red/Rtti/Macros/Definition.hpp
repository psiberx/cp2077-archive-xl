#pragma once

#define X_RTTI_EXPAND(X) X
#define X_RTTI_OVERLOAD(_1, _2, _3, N, ...) N

#define RTTI_IMPL_TYPEINFO(_class) \
    Red::CClass* GetNativeType() \
    { \
        return Red::GetClass<_class>(); \
    }

#define RTTI_IMPL_ALLOCATOR(_allocator) \
    using AllocatorType = _allocator; \
    static_assert(Red::Detail::IsAllocator<_allocator>, #_allocator " is not a valid allocator type"); \
    Red::Memory::IAllocator* GetAllocator() \
    { \
        return _allocator::Get(); \
    }

#define RTTI_DECLARE_FRIENDS(_class) \
    friend class Red::RTTITypeBuilder<_class>; \
    friend class Red::ClassDescriptor<_class>;

#define RTTI_DEFINE_CLASS(...) \
    X_RTTI_EXPAND(X_RTTI_OVERLOAD(__VA_ARGS__, X_RTTI_DEF_CLASS_3, X_RTTI_DEF_CLASS_2, X_RTTI_DEF_CLASS_1)(__VA_ARGS__))

#define X_RTTI_DEF_CLASS_1(_class) \
    template<> \
    struct Red::RTTITypeBuilder<_class> : Red::ClassBuilder<_class> \
    { \
        using Type = _class; \
        using Descriptor = Red::ClassDescriptor<_class>; \
        static constexpr auto Name() \
        { \
            return nameof::nameof_short_type<Type>(); \
        } \
    };

#define X_RTTI_DEF_CLASS_2(_class, _desc) \
    template<> \
    struct Red::RTTITypeBuilder<_class> : Red::ClassBuilder<_class> \
    { \
        using Type = _class; \
        using Descriptor = Red::ClassDescriptor<_class>; \
        static constexpr auto Name() \
        { \
            return nameof::nameof_short_type<Type>(); \
        } \
        static void Describe(Descriptor* type) \
        { \
            _desc; \
        } \
    };

#define X_RTTI_DEF_CLASS_3(_class, _name, _desc) \
    template<> \
    struct Red::RTTITypeBuilder<_class> : Red::ClassBuilder<_class> \
    { \
        using Type = _class; \
        using Descriptor = Red::ClassDescriptor<_class>; \
        static constexpr auto Name() \
        { \
            return _name; \
        } \
        static void Describe(Descriptor* type) \
        { \
            _desc; \
        } \
    };

#define RTTI_EXPAND_CLASS(...) \
    X_RTTI_EXPAND(X_RTTI_OVERLOAD(__VA_ARGS__, X_RTTI_EXT_CLASS_3, X_RTTI_EXT_CLASS_2, X_RTTI_EXT_CLASS_1)(__VA_ARGS__))

#define X_RTTI_EXT_CLASS_2(_class, _desc) \
    template<> \
    struct Red::RTTITypeBuilder<_class> : Red::ClassExpansion<_class, _class> \
    { \
        using Type = _class; \
        using Descriptor = Red::ClassDescriptor<_class>; \
        static void Describe(Descriptor* type) \
        { \
            _desc; \
        } \
    };

#define X_RTTI_EXT_CLASS_3(_expansion, _class, _desc) \
    template<> \
    struct Red::RTTITypeBuilder<_expansion> : Red::ClassExpansion<_expansion, _class> \
    { \
        using Type = _expansion; \
        using Descriptor = Red::ClassDescriptor<_class>; \
        static void Describe(Descriptor* type) \
        { \
            _desc; \
        } \
    };

#define RTTI_ABSTRACT() \
    type->MarkAbstract()

#define RTTI_PARENT(_parent) \
    type->SetParent<_parent>()

#define RTTI_ALIAS(_alias) \
    type->SetAlias(_alias)

#define RTTI_METHOD(...) \
    X_RTTI_EXPAND(X_RTTI_OVERLOAD(__VA_ARGS__, X_RTTI_METHOD_3, X_RTTI_METHOD_2, X_RTTI_METHOD_1)(__VA_ARGS__))

#define X_RTTI_METHOD_1(_method) \
    type->AddFunction<&Type::_method>(#_method)

#define X_RTTI_METHOD_2(_method, _name) \
    type->AddFunction<&Type::_method>(_name)

#define RTTI_CALLBACK(...) \
    X_RTTI_EXPAND(X_RTTI_OVERLOAD(__VA_ARGS__, X_RTTI_CALLBACK_3, X_RTTI_CALLBACK_2, X_RTTI_CALLBACK_1)(__VA_ARGS__))

#define X_RTTI_CALLBACK_1(_method) \
    type->AddFunction<&Type::_method>(#_method, {.isEvent = true})

#define X_RTTI_CALLBACK_2(_method, _name) \
    type->AddFunction<&Type::_method>(_name, {.isEvent = true})

#define RTTI_PROPERTY(...) \
    X_RTTI_EXPAND(X_RTTI_OVERLOAD(__VA_ARGS__, X_RTTI_PROPERTY_3, X_RTTI_PROPERTY_2, X_RTTI_PROPERTY_1)(__VA_ARGS__))

#define X_RTTI_PROPERTY_1(_property) \
    type->AddProperty<&Type::_property>(#_property)

#define X_RTTI_PROPERTY_2(_property, _name) \
    type->AddProperty<&Type::_property>(_name)

#define RTTI_DEFINE_ENUM(...) \
    X_RTTI_EXPAND(X_RTTI_OVERLOAD(__VA_ARGS__, X_RTTI_DEF_ENUM_3, X_RTTI_DEF_ENUM_2, X_RTTI_DEF_ENUM_1)(__VA_ARGS__))

#define X_RTTI_DEF_ENUM_1(_enum) \
    template<> \
    struct Red::RTTITypeBuilder<_enum> : Red::EnumBuilder<_enum, false> \
    { \
        using Type = _enum; \
        using Descriptor = Red::EnumDescriptor<_enum>; \
        static constexpr auto Name() \
        { \
            return nameof::nameof_short_type<Type>(); \
        } \
    };

#define X_RTTI_DEF_ENUM_2(_enum, _name) \
    template<> \
    struct Red::RTTITypeBuilder<_enum> : Red::EnumBuilder<_enum, false> \
    { \
        using Type = _enum; \
        using Descriptor = Red::EnumDescriptor<_enum>; \
        static constexpr auto Name() \
        { \
            return _name; \
        } \
    };

#define RTTI_DEFINE_FLAGS(...) \
    X_RTTI_EXPAND(X_RTTI_OVERLOAD(__VA_ARGS__, X_RTTI_DEF_FLAGS_3, X_RTTI_DEF_FLAGS_2, X_RTTI_DEF_FLAGS_1)(__VA_ARGS__))

#define X_RTTI_DEF_FLAGS_1(_enum) \
    template<> \
    struct Red::RTTITypeBuilder<_enum> : Red::EnumBuilder<_enum, true> \
    { \
        using Type = _enum; \
        using Descriptor = Red::EnumDescriptor<_enum>; \
        static constexpr auto Name() \
        { \
            return nameof::nameof_short_type<Type>(); \
        } \
    };

#define X_RTTI_DEF_FLAGS_2(_enum, _name) \
    template<> \
    struct Red::RTTITypeBuilder<_enum> : Red::EnumBuilder<_enum, true> \
    { \
        using Type = _enum; \
        using Descriptor = Red::EnumDescriptor<_enum>; \
        static constexpr auto Name() \
        { \
            return _name; \
        } \
    };

#define RTTI_EXPAND_ENUM(...) \
    X_RTTI_EXPAND(X_RTTI_OVERLOAD(__VA_ARGS__, X_RTTI_EXP_ENUM_3, X_RTTI_EXP_ENUM_2, X_RTTI_EXP_ENUM_1)(__VA_ARGS__))

#define X_RTTI_EXP_ENUM_1(_enum) \
    template<> \
    struct Red::RTTITypeBuilder<_enum> : Red::EnumExpansion<_enum, _enum, false> \
    { \
        using Type = _enum; \
        using Descriptor = Red::EnumDescriptor<_enum>; \
    };

#define X_RTTI_EXP_ENUM_2(_expansion, _enum) \
    template<> \
    struct Red::RTTITypeBuilder<_expansion> : Red::EnumExpansion<_expansion, _enum, false> \
    { \
        using Type = _expansion; \
        using Descriptor = Red::EnumDescriptor<_enum>; \
    };

#define RTTI_EXPAND_FLAGS(...) \
    X_RTTI_EXPAND(X_RTTI_OVERLOAD(__VA_ARGS__, X_RTTI_EXP_FLAGS_3, X_RTTI_EXP_FLAGS_2, X_RTTI_EXP_FLAGS_1)(__VA_ARGS__))

#define X_RTTI_EXP_FLAGS_1(_enum) \
    template<> \
    struct Red::RTTITypeBuilder<_enum> : Red::EnumExpansion<_enum, _enum, true> \
    { \
        using Type = _enum; \
        using Descriptor = Red::EnumDescriptor<_enum>; \
    };

#define X_RTTI_EXP_FLAGS_2(_expansion, _enum) \
    template<> \
    struct Red::RTTITypeBuilder<_expansion> : Red::EnumExpansion<_expansion, _enum, true> \
    { \
        using Type = _expansion; \
        using Descriptor = Red::EnumDescriptor<_enum>; \
    };

#define RTTI_OPTION() \
    static_assert(false, "Not Implemented")

#define RTTI_DEFINE_GLOBALS(...) \
    X_RTTI_EXPAND(X_RTTI_OVERLOAD(__VA_ARGS__, X_RTTI_DEF_GLOB_3, X_RTTI_DEF_GLOB_2, X_RTTI_DEF_GLOB_1)(__VA_ARGS__))

#define X_RTTI_DEF_GLOB_1(_desc) \
    template<> \
    struct Red::RTTIScopeBuilder<Red::AutoScope()> : Red::GlobalBuilder<Red::AutoScope()> \
    { \
        using Descriptor = Red::GlobalDescriptor; \
        static void Describe(Descriptor* rtti) \
        { \
            _desc; \
        } \
    };

#define X_RTTI_DEF_GLOB_2(_namespace, _desc) \
    template<> \
    struct Red::RTTIScopeBuilder<Red::AutoScope()> : Red::GlobalBuilder<Red::AutoScope()> \
    { \
        using Descriptor = Red::GlobalDescriptor; \
        static void Describe(Descriptor* rtti) \
        { \
            using namespace _namespace; \
            _desc; \
        } \
    };

#define RTTI_FUNCTION(...) \
    X_RTTI_EXPAND(X_RTTI_OVERLOAD(__VA_ARGS__, X_RTTI_FUNCTION_3, X_RTTI_FUNCTION_2, X_RTTI_FUNCTION_1)(__VA_ARGS__))

#define X_RTTI_FUNCTION_1(_func) \
    rtti->AddFunction<&_func>(NAMEOF(_func).data())

#define X_RTTI_FUNCTION_2(_func, _name) \
    rtti->AddFunction<&_func>(_name)

#define RTTI_OP() \
    static_assert(false, "Not Implemented")

#define RTTI_CAST() \
    static_assert(false, "Not Implemented")

#define RTTI_REGISTER(_handler) \
    template<> \
    struct Red::RTTIScopeBuilder<Red::AutoScope()> : Red::GlobalBuilder<Red::AutoScope()> \
    { \
        using Descriptor = Red::GlobalDescriptor; \
        static void Register(Descriptor* rtti) \
        { \
            _handler; \
        } \
    };

#define RTTI_DESCRIBE(_handler) \
    template<> \
    struct Red::RTTIScopeBuilder<Red::AutoScope()> : Red::GlobalBuilder<Red::AutoScope()> \
    { \
        using Descriptor = Red::GlobalDescriptor; \
        static void Describe(Descriptor* rtti) \
        { \
            _handler; \
        } \
    };
