#pragma once

#define RTTI_TYPE_NAME(_type, _name) \
    template<> \
    struct Red::TypeNameMapping<_type> : public std::true_type \
    { \
        static constexpr auto name = _name; \
    };

#define RTTI_TYPE_PREFIX(_type, _prefix) \
    template<typename A> \
    struct Red::TypePrefixMapping<_type<A>> : public std::true_type \
    { \
        static constexpr auto prefix = _prefix; \
    };
