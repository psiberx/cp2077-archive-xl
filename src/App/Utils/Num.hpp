#pragma once

namespace App
{
template<typename T>
requires std::is_integral_v<T>
bool ParseInt(const char* aIn, size_t aLength, T& aOut, const int aRadix = 10)
{
    using Temp = std::conditional_t<std::is_signed_v<T>, int64_t, uint64_t>;

    Temp out;
    char* end;

    if constexpr (std::is_signed_v<T>)
        out = std::strtoll(aIn, &end, aRadix);
    else
        out = std::strtoull(aIn, &end, aRadix);

    if (end != aIn + aLength)
        return false;

    aOut = static_cast<T>(out);
    return true;
}

template<typename T>
requires std::is_integral_v<T>
bool ParseInt(const std::string& aIn, T& aOut, const int aRadix = 10)
{
    return ParseInt(aIn.c_str(), aIn.size(), aOut, aRadix);
}

template<typename T>
requires std::is_integral_v<T>
T ParseInt(const std::string& aIn, const int aRadix = 10)
{
    using Temp = std::conditional_t<std::is_signed_v<T>, int64_t, uint64_t>;

    Temp out;

    if constexpr (std::is_signed_v<T>)
        out = std::strtoll(aIn.c_str(), nullptr, aRadix);
    else
        out = std::strtoull(aIn.c_str(), nullptr, aRadix);

    return static_cast<T>(out);
}

template<typename T>
requires std::is_floating_point_v<T>
bool ParseFloat(const std::string& aIn, T& aOut, const char* aSuffix = nullptr)
{
    char* end;

    if constexpr (std::is_same_v<T, float>)
        aOut = std::strtof(aIn.c_str(), &end);
    else if constexpr (std::is_same_v<T, double>)
        aOut = std::strtod(aIn.c_str(), &end);
    else if constexpr (std::is_same_v<T, long double>)
        aOut = std::strtold(aIn.c_str(), &end);

    if (end != aIn.c_str() + aIn.size())
    {
        return aSuffix && strcmp(end, aSuffix) == 0;
    }

    return true;
}

inline bool IsNumeric(const std::string& aIn, size_t aStart = 0)
{
    return aIn.find_first_not_of("0123456789", aStart) == std::string::npos;
}

template<typename T>
requires std::is_integral_v<T>
inline std::string ToHex(T aValue)
{
    constexpr auto max = 2 * sizeof(uint64_t);
    constexpr auto len = 2 * sizeof(aValue);
    static char buffer[max + 1];
    snprintf(buffer, max + 1, "%016X", aValue);
    return {buffer + (max - len), len};
}
}
