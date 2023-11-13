#pragma once

#define RTTI_MAP_RUNTIME_SYSTEM(_type, _offset) \
    template<> \
    struct Red::RuntimeSystemMapping<_type> : public std::true_type \
    { \
        static constexpr auto offset = _offset; \
    };
