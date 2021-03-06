// Copyright 2019-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the obake library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cstdint>
#include <exception>

#include <mp++/integer.hpp>

#include <obake/polynomials/packed_monomial.hpp>

#include "sparse.hpp"

using namespace obake;
using namespace obake_benchmark;

int main()
{
    try {
        sparse_benchmark<packed_monomial<std::uint64_t>, mppp::integer<2>>(20);
    } catch (std::exception &e) {
        std::cerr << e.what();
    }
}
