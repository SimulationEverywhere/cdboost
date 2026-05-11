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

#include <catch2/catch_test_macros.hpp>
#include <cdboost/pdevs/basic_models/processor.hpp>
#include <cmath>

using namespace cdboost::pdevs::basic_models;

using Time    = double;
using Message = int;

TEST_CASE("processor handles single jobs with various delays", "[processor]") {
    for (int i = 0; i < 10; i++) {
        processor<Time, Message> p{static_cast<Time>(i)};
        CHECK(std::isinf(p.advance()));
        p.external({i}, Time{1});
        CHECK(p.advance() == Time(i));
        CHECK(p.out()[0] == i);
        p.internal();
        CHECK(std::isinf(p.advance()));
    }
}

TEST_CASE("processor handles sequential jobs", "[processor]") {
    for (int i = 0; i < 10; i++) {
        processor<Time, Message> p{static_cast<Time>(i)};
        CHECK(std::isinf(p.advance()));
        for (int j = 0; j <= i; j++) {
            p.external({j}, Time{1});
            CHECK(p.advance() == Time(i));
            CHECK(p.out()[0] == j);
            p.internal();
            CHECK(std::isinf(p.advance()));
        }
    }
}

TEST_CASE("processor returns multiple queued jobs one at a time", "[processor]") {
    processor<Time, Message> p{Time{1}};
    CHECK(std::isinf(p.advance()));
    p.external({1, 2, 3, 4}, Time{0});
    for (int i = 0; i < 4; i++) {
        CHECK(p.advance() == Time{1});
        CHECK(p.out().size() == 1);
        p.internal();
    }
    CHECK(std::isinf(p.advance()));
}
