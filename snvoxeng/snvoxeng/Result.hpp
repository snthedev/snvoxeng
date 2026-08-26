#pragma once

#include <snassert/snassert.hpp>

#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

namespace sn::voxeng
{
    // Generic error carrier for physically recoverable failures (see the
    // project error-handling policy): a domain-specific integral code plus an
    // optional human-readable detail message. A default-constructed Error is
    // 'success' and must never be stored inside a failing Result.
    struct Error
    {
        uint32_t code{ 0 };
        std::string message;
    };

    // Minimal expected-like container for functions that can fail in a
    // recoverable way. API-contract violations are NOT reported through
    // Result - those belong to snassert (see the policy).
    //
    // Accessing the wrong side (value() on a failure, error() on a success)
    // is itself a contract violation: snassert catches it in Debug builds,
    // and Release falls back to std::bad_variant_access instead of UB.
    template <typename T>
    class [[nodiscard]] Result
    {
        std::variant<T, Error> m_data;

        bool holdsValue() const noexcept { return m_data.index() == 0u; }

    public:
        Result(T value) noexcept(std::is_nothrow_move_constructible_v<T>)
            : m_data(std::move(value))
        {
        }
        Result(Error error) noexcept
            : m_data(std::move(error))
        {
        }

        bool isOk() const noexcept { return holdsValue(); }
        explicit operator bool() const noexcept { return holdsValue(); }

        const T& value() const &
        {
            snassert(holdsValue(), "Result::value() called on a failure",
                "Check Result::isOk() before accessing the value");
            return std::get<T>(m_data);
        }
        T& value() &
        {
            snassert(holdsValue(), "Result::value() called on a failure",
                "Check Result::isOk() before accessing the value");
            return std::get<T>(m_data);
        }

        // Moves the value out (invalidates the stored instance).
        T&& take()
        {
            snassert(holdsValue(), "Result::take() called on a failure",
                "Check Result::isOk() before taking the value");
            return std::move(std::get<T>(m_data));
        }

        const Error& error() const
        {
            snassert(!holdsValue(), "Result::error() called on a success",
                "Check Result::isOk() before accessing the error");
            return std::get<Error>(m_data);
        }

        const T* operator->() const
        {
            snassert(holdsValue(), "Result operator-> called on a failure",
                "Check Result::isOk() before dereferencing");
            return &std::get<T>(m_data);
        }
        const T& operator*() const & { return value(); }
    };
}