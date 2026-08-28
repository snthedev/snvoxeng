#pragma once

#include <snvoxeng/snvoxeng/utils/LeTypes.hpp>

#include <type_traits>
#include <cstring>
#include <cassert>
#include <stdexcept>
#include <span>

namespace sn::voxeng::io
{
    template <typename ByteT>
    class BFView;

    template <typename ByteT>
    class BFView
    {
        static_assert(
            std::is_same_v<std::remove_cv_t<ByteT>, std::byte>
            || std::is_same_v<std::remove_cv_t<ByteT>, uint8_t>,
            "BFView supports only byte types."
            );

    public:
        using value_type = ByteT;
        using size_type = size_t;
        using pointer = ByteT*;
        using const_pointer = const ByteT*;

    private:
        pointer m_data{};
        size_type m_size{};

        constexpr void check_range(size_type offset, size_type count) const
        {
            if (offset <= m_size && count <= m_size - offset)
                return;

            assert(false && "BFView: range is out of bounds.");
            throw std::out_of_range("BFView: range is out of bounds.");
        }

    public:
        constexpr BFView() noexcept = default;

        constexpr BFView(pointer data, size_type size) noexcept
            : m_data(data)
            , m_size(size)
        {
        }

        template <size_t N>
        constexpr BFView(ByteT(&array)[N]) noexcept
            : m_data(array)
            , m_size(N)
        {
        }

        constexpr pointer data() noexcept { return m_data; }
        constexpr const_pointer data() const noexcept { return m_data; }

        constexpr size_type size() const noexcept { return m_size; }

        constexpr bool empty() const noexcept { return m_size == 0; }

        constexpr std::span<ByteT> bytes() noexcept { return { m_data, m_size }; }
        constexpr std::span<const std::remove_const_t<ByteT>> bytes() const noexcept { return { m_data, m_size }; }


        // ================================================================
        // View manipulation
        // ================================================================

        constexpr BFView subview(size_type offset, size_type count) const
        {
            check_range(offset, count);
            return { m_data + offset, count };
        }

        constexpr BFView subview(size_type offset) const
        {
            check_range(offset, m_size - offset);
            return { m_data + offset, m_size - offset };
        }

        constexpr void remove_prefix(size_type count)
        {
            check_range(0, count);

            m_data += count;
            m_size -= count;
        }

        constexpr void remove_suffix(size_type count)
        {
            check_range(m_size - count, count);

            m_size -= count;
        }

        constexpr void reset(pointer data, size_type size) noexcept
        {
            m_data = data;
            m_size = size;
        }


        // ================================================================
        // Reading
        // ================================================================

        template <typename T> requires std::is_arithmetic_v<T>
        T read(size_type offset) const
        {
            check_range(offset, sizeof(T));

            le_type<T> value{};
            std::memcpy(&value, m_data + offset, sizeof(value));

            return value;
        }

        // same as:
        //  read(sizeof(value), value);
        //  remove_prefix(sizeof(value));
        template <typename T> requires std::is_arithmetic_v<T>
        T read_front()
        {
            T value = read<T>(0);
            remove_prefix(sizeof(T));
            return value;
        }


        // ================================================================
        // Writing
        // ================================================================

        template <typename T> requires (!std::is_const_v<ByteT> && std::is_arithmetic_v<T>)
        void write(size_type offset, T value)
        {
            check_range(offset, sizeof(T));

            const le_type<T> le_value{ value };
            std::memcpy(m_data + offset, &le_value, sizeof(le_value));
        }

        // same as:
        //  write(sizeof(value), value);
        //  remove_prefix(sizeof(value));
        template <typename T> requires (!std::is_const_v<ByteT>&& std::is_arithmetic_v<T>)
        void write_front(T value)
        {
            write(0, value);
            remove_prefix(sizeof(T));
        }
    };


    typedef BFView<const uint8_t> BFViewRO;
    typedef BFView<uint8_t> BFViewRW;


    template <typename T>
    concept BFViewRAble = std::is_same_v<std::remove_cv_t<T>, BFViewRO> || std::is_same_v<std::remove_cv_t<T>, BFViewRW>;

    template <typename T>
    concept BFViewWAble = std::is_same_v<std::remove_cv_t<T>, BFViewRW>;
}