#pragma once

#define RTTI_MAP_TYPE_NAME(_type, _name) \
    template<> \
    struct Red::TypeNameMapping<_type> : public std::true_type \
    { \
        static constexpr auto name = _name; \
    };

#define RTTI_MAP_TYPE_PREFIX(_type, _prefix) \
    template<typename A> \
    struct Red::TypePrefixMapping<_type<A>> : public std::true_type \
    { \
        static constexpr auto prefix = _prefix; \
    };

#define RTTI_MAP_TYPE_PROXY(_type) \
    template<typename A, auto... Args> \
    struct Red::TypeProxyMapping<_type<A, Args...>> : public std::true_type \
    { \
        using type = A; \
    };

#define RTTI_TYPE_NAME_STR(_type) Red::GetTypeNameStr<_type>()

#define RTTI_FUNC_NAME_STR(...) []() constexpr noexcept { \
  constexpr auto _name = ::nameof::detail::pretty_name(#__VA_ARGS__); \
  return ::Red::Detail::MakeConstStr<_name.size()>(_name.data()); }()

#define RTTI_PROP_NAME_STR(...) []() constexpr noexcept { \
  constexpr auto _name = ::nameof::detail::pretty_name(#__VA_ARGS__); \
  constexpr auto _clean = ::Red::Detail::RemoveMemberPrefix(_name); \
  return ::Red::Detail::MakeConstStr<_clean.size()>(_clean.data()); }()

#define RTTI_ENUM_NAME_STR(...) ::nameof::nameof_enum<__VA_ARGS__>()

#define RTTI_TYPE_NAME(_type) ::Red::GetTypeName<_type>()
#define RTTI_FUNC_NAME(_func) ::Red::CName(RTTI_FUNC_NAME_STR(&_func).data())
#define RTTI_PROP_NAME(_prop) ::Red::CName(RTTI_PROP_NAME_STR(&_prop).data())
#define RTTI_ENUM_NAME(_enum) ::Red::CName(RTTI_ENUM_NAME_STR(_enum).data())
