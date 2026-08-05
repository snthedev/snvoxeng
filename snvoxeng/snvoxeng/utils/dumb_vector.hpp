#pragma once

#include <snassert/snassert.hpp>
#include <cstddef>

namespace sn::voxeng
{
    template<typename T>
    class dumb_vector
    {
        std::byte* m_pBuffer{ nullptr };
        size_t m_capacity{ 0 };
        size_t m_size{ 0 };

        void alloc()
        {
            if (m_capacity == 0) [[unlikely]] return;
            m_pBuffer = static_cast<std::byte*>(::operator new[](
                sizeof(T) * m_capacity,
                std::align_val_t{ alignof(T) }
                ));
        }
        void free() noexcept
        {
            if (!m_pBuffer) [[unlikely]] return;
            clear();
            ::operator delete[](
                m_pBuffer,
                std::align_val_t{ alignof(T) }
                );
            m_pBuffer = nullptr;
            m_capacity = 0;
        }

    public:
        // call reserve() manually
        dumb_vector() noexcept : m_capacity(0) {}
        explicit dumb_vector(size_t capacity) : m_capacity(capacity) { alloc(); }
        ~dumb_vector() noexcept { free(); }

        dumb_vector(const dumb_vector&) = delete;
        dumb_vector& operator=(const dumb_vector&) = delete;
        dumb_vector(dumb_vector&& other) noexcept
            : m_pBuffer(std::exchange(other.m_pBuffer, nullptr))
            , m_capacity(std::exchange(other.m_capacity, 0))
            , m_size(std::exchange(other.m_size, 0))
        {
        }
        dumb_vector& operator=(dumb_vector&& other) noexcept
        {
            if (this != &other)
            {
                free();
                m_pBuffer = std::exchange(other.m_pBuffer, nullptr);
                m_capacity = std::exchange(other.m_capacity, 0);
                m_size = std::exchange(other.m_size, 0);
            }
            return *this;
        }

        // reserve() CLEARS the entire vector
        void reserve(size_t capacity)
        {
            free();
            m_capacity = capacity;
            alloc();
        }

        template<typename... Args>
        T& emplace_back(Args&&... args)
        {
            snassert(m_size < m_capacity, "Out of dumb_vector capacity", "dumb_vector::size() can not be greater than dumb_vector::capacity()");

            ::new (static_cast<void*>(begin() + m_size++)) T(std::forward<Args>(args)...);
            return back();
        }

        template<typename Builder, typename... Args>
        T& emplace_builder(Builder&& builder, Args&&... args)
        {
            snassert(m_size < m_capacity, "Out of dumb_vector capacity", "dumb_vector::size() can not be greater than dumb_vector::capacity()");

            ::new (static_cast<void*>(begin() + m_size++)) T(builder.build(std::forward<Args>(args)...));
            return back();
        }

        void clear() noexcept
        {
            for (T* ptr = begin(); ptr != end(); ++ptr) ptr->~T();
            m_size = 0;
        }

        const T& operator[](size_t idx) const
        {
            snassert(idx < m_size, "Out of dumb_vector size", "idx must be less than dumb_vector::size()");
            return *(begin() + idx);
        }
        T& operator[](size_t idx)
        {
            snassert(idx < m_size, "Out of dumb_vector size", "idx must be less than dumb_vector::size()");
            return *(begin() + idx);
        }

        operator std::span<const T>() const noexcept
        {
            return { begin(), m_size };
        }
        operator std::span<T>() noexcept
        {
            return { begin(), m_size };
        }

        T* begin() noexcept { return reinterpret_cast<T*>(m_pBuffer); }
        T* end() noexcept { return begin() + m_size; }
        const T* begin() const noexcept { return reinterpret_cast<const T*>(m_pBuffer); }
        const T* end() const noexcept { return begin() + m_size; }

        T& front() noexcept
        {
            snassert(!empty(), "Out of dumb_vector", "front() called on empty dumb_vector");
            return operator[](0);
        }
        T& back() noexcept
        {
            snassert(!empty(), "Out of dumb_vector", "back() called on empty dumb_vector");
            return operator[](m_size - 1u);
        }
        const T& front() const noexcept
        {
            snassert(!empty(), "Out of dumb_vector", "front() called on empty dumb_vector");
            return operator[](0);
        }
        const T& back() const noexcept
        {
            snassert(!empty(), "Out of dumb_vector", "back() called on empty dumb_vector");
            return operator[](m_size - 1u);
        }

        size_t capacity() const noexcept { return m_capacity; }
        size_t size() const noexcept { return m_size; }
        bool empty() const noexcept { return m_size == 0; }
    };
}
