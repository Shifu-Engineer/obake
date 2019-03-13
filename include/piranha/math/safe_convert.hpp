// Copyright 2019 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the piranha library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef PIRANHA_MATH_SAFE_CONVERT_HPP
#define PIRANHA_MATH_SAFE_CONVERT_HPP

#include <cstddef>
#include <type_traits>
#include <utility>

#include <mp++/integer.hpp>

#include <piranha/config.hpp>
#include <piranha/detail/not_implemented.hpp>
#include <piranha/detail/priority_tag.hpp>
#include <piranha/detail/ss_func_forward.hpp>
#include <piranha/type_traits.hpp>

namespace piranha
{

namespace customisation
{

// External customisation point for piranha::safe_convert().
template <typename T, typename U
#if !defined(PIRANHA_HAVE_CONCEPTS)
          ,
          typename = void
#endif
          >
inline constexpr auto safe_convert = not_implemented;

} // namespace customisation

namespace detail
{

// Implementation for C++ integrals.
#if defined(PIRANHA_HAVE_CONCEPTS)
template <typename T, typename U>
    requires CppIntegral<T> && !Const<T> && CppIntegral<U>
#else
template <typename T, typename U,
          ::std::enable_if_t<::std::conjunction_v<is_cpp_integral<T>, ::std::negation<is_const<T>>, is_cpp_integral<U>>,
                             int> = 0>
#endif
    constexpr bool safe_convert(T &out, const U &n_orig) noexcept
{
    // Small helpers to get the min/max values of type T.
    // For bool, they will cast the return type to unsigned in
    // order to avoid compiler warnings.
    [[maybe_unused]] constexpr auto Tmin = []() {
        if constexpr (::std::is_same_v<T, bool>) {
            return 0u;
        } else {
            return ::std::get<0>(detail::limits_minmax<T>);
        }
    }();
    constexpr auto Tmax = []() {
        if constexpr (::std::is_same_v<T, bool>) {
            return 1u;
        } else {
            return ::std::get<1>(detail::limits_minmax<T>);
        }
    }();

    // Promote n to unsigned, if it is of type bool, in order
    // to avoid compiler warnings.
    const auto n = [&n_orig]() {
        if constexpr (std::is_same_v<U, bool>) {
            return static_cast<unsigned>(n_orig);
        } else {
            return n_orig;
        }
    }();

    if constexpr (is_signed_v<T> == is_signed_v<U>) {
        // Same signedness, we can use direct comparisons.
        if (n >= Tmin && n <= Tmax) {
            out = static_cast<T>(n);
            return true;
        }
        return false;
    } else {
        if constexpr (is_signed_v<T>) {
            // T signed, U unsigned.
            if (n <= static_cast<make_unsigned_t<T>>(Tmax)) {
                out = static_cast<T>(n);
                return true;
            }
            return false;
        } else {
            // T unsigned, U signed.
            if (n < U(0) || static_cast<make_unsigned_t<U>>(n) > Tmax) {
                return false;
            }
            out = static_cast<T>(n);
            return true;
        }
    }
}

// Implementations for mppp::integer - C++ integrals.
#if defined(PIRANHA_HAVE_CONCEPTS)
template <::std::size_t SSize, typename T>
requires CppIntegral<T>
#else
template <::std::size_t SSize, typename T, ::std::enable_if_t<is_cpp_integral_v<T>, int> = 0>
#endif
    inline bool safe_convert(::mppp::integer<SSize> &n, const T &m)
{
    n = m;
    return true;
}

#if defined(PIRANHA_HAVE_CONCEPTS)
template <typename T, ::std::size_t SSize>
requires CppIntegral<T> && !Const<T>
#else
template <typename T, ::std::size_t SSize,
          ::std::enable_if_t<::std::conjunction_v<is_cpp_integral<T>, ::std::negation<is_const<T>>>, int> = 0>
#endif
    inline bool safe_convert(T &n, const ::mppp::integer<SSize> &m)
{
    return ::mppp::get(n, m);
}

// Highest priority: explicit user override in the external customisation namespace.
template <typename T, typename U>
constexpr auto safe_convert_impl(T &&x, U &&y, priority_tag<1>)
    PIRANHA_SS_FORWARD_FUNCTION((customisation::safe_convert<T &&, U &&>)(::std::forward<T>(x), ::std::forward<U>(y)));

// Unqualified function call implementation.
template <typename T, typename U>
constexpr auto safe_convert_impl(T &&x, U &&y, priority_tag<0>)
    PIRANHA_SS_FORWARD_FUNCTION(safe_convert(::std::forward<T>(x), ::std::forward<U>(y)));

} // namespace detail

inline constexpr auto safe_convert =
    [](auto &&x, auto &&y) PIRANHA_SS_FORWARD_LAMBDA(static_cast<bool>(detail::safe_convert_impl(
        ::std::forward<decltype(x)>(x), ::std::forward<decltype(y)>(y), detail::priority_tag<1>{})));

} // namespace piranha

#endif
