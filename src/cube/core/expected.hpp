#pragma once

#include <optional>

namespace cube::core
{

template<typename T, typename E>
class basic_expected
{
public:
    basic_expected(T value) :
        value_(std::move(value))
    { }

    basic_expected(E error) :
        error_(std::move(error))
    { }

    explicit operator bool() const { return value_.has_value(); }

    T && take() { check_value(); return std::move(*value_); }
    T const * operator->() const { check_value(); return value_.operator->(); }
    T * operator->() { check_value(); return value_.operator->(); }
    T const & operator*() const { check_value(); return *value_; }
    T & operator*() { check_value(); return *value_; }
    E const & error() const { check_error(); return *error_; }
    E & error() { check_error(); return *error_; }

private:
    static_assert(!std::is_same_v<E, void>, "E must not be void");

    void check_value() const
    {
        if (!value_)
            throw std::runtime_error("Tried to access value but has error");
    }

    void check_error() const
    {
        if (!error_)
            throw std::runtime_error("Tried to access error but has expected value");
    }

    std::optional<T> value_;
    std::optional<E> error_;
};

template<typename E>
class basic_expected<void, E>
{
public:
    explicit operator bool() const { return !error_.has_value(); }

    E const & error() const { check_error(); return *error_; }
    E & error() { check_error(); return *error_; }

private:
    static_assert(!std::is_same_v<E, void>, "E must not be void");

    void check_error() const
    {
        if (!error_)
            throw std::runtime_error("Tried to access error but has expected value");
    }

    std::optional<E> error_;
};

struct unexpected_error { std::string what; };
template<typename T>
using expected_or_error = basic_expected<T, unexpected_error>;
using void_or_error = basic_expected<void, unexpected_error>;

}
