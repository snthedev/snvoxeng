#pragma once

#include <snassert/snassert.hpp>
#include <cstddef>

namespace sn::voxeng
{
    template<typename T>
    class dumb_vector
    {
        alignas(alignof(T)) std::byte* m_pBuffer = nullptr;
        size_t m_capacity = 0;
        size_t m_size = 0;

        void alloc() { m_pBuffer = new std::byte[sizeof(T) * m_capacity]; }
        void free() noexcept { clear(); delete[] m_pBuffer; }

    public:
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
            snassert(m_size < m_capacity, "Size cannot be greater than capasity. It's dumb vector", "");

            T* ptr = reinterpret_cast<T*>(m_pBuffer + m_size * sizeof(T));
            ::new (static_cast<void*>(ptr)) T(std::forward<Args>(args)...);
            m_size++;
            return *ptr;
        }

        template<typename Builder, typename... Args>
        T& emplace_builder(Builder&& builder, Args&&... args)
        {
            snassert(m_size < m_capacity, "Size cannot be greater than capasity. It's dumb vector", "");

            T* ptr = reinterpret_cast<T*>(m_pBuffer + m_size * sizeof(T));
            ::new (static_cast<void*>(ptr)) T(builder.build(std::forward<Args>(args)...));
            m_size++;
            return *ptr;
        }

        void clear() noexcept
        {
            for (size_t i = 0; i < m_size; ++i)
                reinterpret_cast<T*>(m_pBuffer + i * sizeof(T))->~T();
            m_size = 0;
        }

        const T& operator[](size_t idx) const { return *reinterpret_cast<T*>(m_pBuffer + idx * sizeof(T)); }
        T& operator[](size_t idx) { return *reinterpret_cast<T*>(m_pBuffer + idx * sizeof(T)); }

        operator std::span<const T>() const noexcept
        {
            return std::span<const T>{ reinterpret_cast<const T*>(m_pBuffer), m_size };
        }
        operator std::span<T>() noexcept
        {
            return std::span<T>{ reinterpret_cast<T*>(m_pBuffer), m_size };
        }

        size_t size() const noexcept { return m_size; }
    };
}