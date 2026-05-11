/**
 * Copyright (c) 2013-present, Damian Vicino
 * Carleton University, Universite de Nice-Sophia Antipolis
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <any>
#include <catch2/catch_test_macros.hpp>
#include <cdboost/pdevs/basic_models/infinite_counter.hpp>
#include <cmath>
#include <vector>

using namespace cdboost::pdevs::basic_models;

using Time    = double;
using Message = std::any;

TEST_CASE("infinite_counter counts and resets on zero input", "[infinite_counter]") {
    infinite_counter<Time, Message> ic;
    CHECK(std::isinf(ic.advance()));

    ic.external(std::vector<Message>{1, 2, 3, 4, 5, 6}, Time{1});
    CHECK(std::isinf(ic.advance()));

    ic.external(std::vector<Message>{7, 8, 9, 0}, Time{1});
    CHECK(ic.advance() == Time(0));
    CHECK(std::any_cast<int>(ic.out()[0]) == 9);

    ic.confluence(std::vector<std::any>({0, 1, 2, 3}), Time{0});
    CHECK(ic.advance() == Time(0));
    CHECK(std::any_cast<int>(ic.out()[0]) == 3);

    ic.internal();
    CHECK(std::isinf(ic.advance()));
}

TEST_CASE("infinite_counter accumulates across multiple external calls", "[infinite_counter]") {
    infinite_counter<Time, Message> ic;
    CHECK(std::isinf(ic.advance()));

    for (int i = 1; i < 11; i++) {
        for (int j = 0; j < i; j++) {
            ic.external(std::vector<std::any>{1, 2, 3}, Time{1});
            CHECK(std::isinf(ic.advance()));
        }
        ic.external(std::vector<std::any>{0}, Time{1});
        CHECK(ic.advance() == Time(0));
        CHECK(std::any_cast<int>(ic.out()[0]) == i * 3);
        ic.internal();
        CHECK(std::isinf(ic.advance()));
    }
}
