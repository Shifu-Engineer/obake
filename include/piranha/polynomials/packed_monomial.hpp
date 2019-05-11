// Copyright 2019 Francesco Biscani (bluescarni@gmail.com)::polynomials
//
// This file is part of the piranha library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef PIRANHA_POLYNOMIALS_PACKED_MONOMIAL_HPP
#define PIRANHA_POLYNOMIALS_PACKED_MONOMIAL_HPP

#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <type_traits>
#include <utility>

#include <piranha/config.hpp>
#include <piranha/math/safe_cast.hpp>
#include <piranha/ranges.hpp>
#include <piranha/symbols.hpp>
#include <piranha/type_traits.hpp>
#include <piranha/utils/bit_packing.hpp>

namespace piranha
{

namespace polynomials
{

#if defined(PIRANHA_HAVE_CONCEPTS)
template <BitPackable T>
#else
template <typename T, typename = ::std::enable_if_t<is_bit_packable_v<T>>>
#endif
class packed_monomial
{
public:
    // Def ctor inits to a monomial with all zero exponents.
    constexpr packed_monomial() : m_value(0) {}
    // Constructor from input iterator and size.
#if defined(PIRANHA_HAVE_CONCEPTS)
    template <typename It>
    requires InputIterator<It> &&SafelyCastable<typename ::std::iterator_traits<It>::reference, T>
#else
    template <
        typename It,
        ::std::enable_if_t<::std::conjunction_v<is_input_iterator<It>,
                                                is_safely_castable<typename ::std::iterator_traits<It>::reference, T>>,
                           int> = 0>
#endif
        constexpr explicit packed_monomial(It it, unsigned n)
    {
        bit_packer<T> bp(n);
        for (auto i = 0u; i < n; ++i, ++it) {
            bp << ::piranha::safe_cast<T>(*it);
        }
        m_value = bp.get();
    }

private:
    struct fwd_it_ctor_tag {
    };
    template <typename It>
    constexpr explicit packed_monomial(fwd_it_ctor_tag, It b, It e)
    {
        bit_packer<T> bp(::piranha::safe_cast<unsigned>(::std::distance(b, e)));
        for (; b != e; ++b) {
            bp << ::piranha::safe_cast<T>(*b);
        }
        m_value = bp.get();
    }

public:
    // Ctor from a pair of forward iterators.
#if defined(PIRANHA_HAVE_CONCEPTS)
    template <typename It>
    requires ForwardIterator<It> &&SafelyCastable<typename ::std::iterator_traits<It>::difference_type, unsigned> &&
        SafelyCastable<typename ::std::iterator_traits<It>::reference, T>
#else
    template <typename It,
              ::std::enable_if_t<::std::conjunction_v<
                                     is_forward_iterator<It>,
                                     is_safely_castable<typename ::std::iterator_traits<It>::difference_type, unsigned>,
                                     is_safely_castable<typename ::std::iterator_traits<It>::reference, T>>,
                                 int> = 0>
#endif
        constexpr explicit packed_monomial(It b, It e) : packed_monomial(fwd_it_ctor_tag{}, b, e)
    {
    }
    // Ctor from forward range.
#if defined(PIRANHA_HAVE_CONCEPTS)
    template <typename Range>
    requires ForwardRange<Range> &&
        SafelyCastable<typename ::std::iterator_traits<range_begin_t<Range>>::difference_type, unsigned> &&
            SafelyCastable<typename ::std::iterator_traits<range_begin_t<Range>>::reference, T>
#else
    template <
        typename Range,
        ::std::enable_if_t<
            ::std::conjunction_v<
                is_forward_range<Range>,
                is_safely_castable<typename ::std::iterator_traits<range_begin_t<Range>>::difference_type, unsigned>,
                is_safely_castable<typename ::std::iterator_traits<range_begin_t<Range>>::reference, T>>,
            int> = 0>
#endif
        constexpr explicit packed_monomial(Range &&r)
        : packed_monomial(fwd_it_ctor_tag{}, ::piranha::begin(::std::forward<Range>(r)),
                          ::piranha::end(::std::forward<Range>(r)))
    {
    }
    // Ctor from init list.
#if defined(PIRANHA_HAVE_CONCEPTS)
    template <typename U>
    requires SafelyCastable<const U &, T>
#else
    template <typename U, ::std::enable_if_t<is_safely_castable_v<const U &, T>, int> = 0>
#endif
        constexpr explicit packed_monomial(::std::initializer_list<U> l)
        : packed_monomial(fwd_it_ctor_tag{}, l.begin(), l.end())
    {
    }
    // Getter for the internal value.
    constexpr const T &get_value() const
    {
        return m_value;
    }

private:
    T m_value;
};

// Implementation of key_is_zero(). A monomial is never zero.
template <typename T>
constexpr bool key_is_zero(const packed_monomial<T> &, const symbol_set &)
{
    return false;
}

// Comparison operators.
// TODO test.
template <typename T>
constexpr bool operator==(const packed_monomial<T> &m1, const packed_monomial<T> &m2)
{
    return m1.get_value() == m2.get_value();
}

template <typename T>
constexpr bool operator!=(const packed_monomial<T> &m1, const packed_monomial<T> &m2)
{
    return m1.get_value() != m2.get_value();
}

// Hash implementation.
// TODO check
// TODO test.
template <typename T>
constexpr ::std::size_t hash(const packed_monomial<T> &m)
{
    const auto h1 = static_cast<::std::size_t>(m.get_value());
    const auto h2 = h1 ^ (h1 + ::std::size_t(0x9e3779b9ul) + (h1 << 6) + (h1 >> 2));
    return h1 + (h2 << (::std::numeric_limits<::std::size_t>::digits - 7));
}

} // namespace polynomials

// Lift to the piranha namespace.
template <typename T>
using packed_monomial = polynomials::packed_monomial<T>;

} // namespace piranha

#endif
