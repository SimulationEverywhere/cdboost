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

#include <boost/rational.hpp>

#include <any>
#include <catch2/catch_test_macros.hpp>
#include <cdboost/pdevs/basic_models/generator.hpp>
#include <cdboost/rational_time.hpp>

using namespace cdboost::pdevs::basic_models;

using Time    = boost::rational<int>;
using Message = std::any;

TEST_CASE("generator initial time advance equals period", "[generator]") {
    generator<Time, Message> a(Time{1});
    CHECK(a.advance() == Time{1});
}

TEST_CASE("generator advance always matches period after internal transitions", "[generator]") {
    for (int i = 1; i < 100; i++) {
        Time t = Time(i);
        generator<Time, Message> a(t);
        CHECK(std::any_cast<int>(a.out()[0]) == 1);
        a.internal();
        CHECK(a.advance() == t);
    }
}

TEST_CASE("generator always outputs the preset value", "[generator]") {
    Time t(1);
    generator<Time, Message> a(t, 5);
    for (int j = 1; j < 100; j++) {
        CHECK(std::any_cast<int>(a.out()[0]) == 5);
        a.internal();
        CHECK(a.advance() == t);
    }
}
