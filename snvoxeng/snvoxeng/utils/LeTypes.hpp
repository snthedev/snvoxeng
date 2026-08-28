#pragma once

#include <cstdint>
#include <type_traits>
#include <concepts>
#include <bit>

template <typename T> requires std::is_arithmetic_v<T>
class alignas(alignof(T)) le_type
{
    std::remove_const_t<T> m_value;

    static constexpr inline T to_le(T val) noexcept
    {
        if constexpr (std::endian::native == std::endian::big)
            return std::byteswap(val);
        return val;
    }

public:
    le_type() = default;
    le_type(T val) noexcept : m_value(to_le(val)) {}
    ~le_type() = default;

    operator T() const noexcept { return to_le(m_value); }

    le_type& operator=(T val) noexcept
    {
        m_value = to_le(val);
        return *this;
    }
};

using le_uint8_t = le_type<uint8_t>;
using le_uint16_t = le_type<uint16_t>;
using le_uint32_t = le_type<uint32_t>;
using le_uint64_t = le_type<uint64_t>;

using le_int8_t = le_type<int8_t>;
using le_int16_t = le_type<int16_t>;
using le_int32_t = le_type<int32_t>;
using le_int64_t = le_type<int64_t>;
