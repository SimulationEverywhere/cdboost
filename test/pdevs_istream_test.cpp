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

#include <algorithm>
#include <any>
#include <catch2/catch_test_macros.hpp>
#include <cdboost/pdevs/basic_models/input_stream.hpp>
#include <cmath>
#include <memory>
#include <sstream>
#include <string>

using namespace cdboost::pdevs::basic_models;

using Time    = double;
using Message = std::any;

TEST_CASE("input_stream single event at time 0", "[input_stream]") {
    auto piss = std::make_shared<std::istringstream>("0 0");
    input_stream<Time, Message, int, int> pf{piss, Time(0)};
    CHECK(pf.advance() == Time(0));
    REQUIRE(pf.out().size() == 1);
    CHECK(std::any_cast<int>(pf.out()[0]) == 0);
    pf.internal();
    CHECK(std::isinf(pf.advance()));
}

TEST_CASE("input_stream multiple events at same time", "[input_stream]") {
    auto piss = std::make_shared<std::istringstream>("0 0 \n 0 1 \n 0 2 ");
    input_stream<Time, Message, int, int> pf{piss, Time(0)};
    CHECK(pf.advance() == Time(0));
    REQUIRE(pf.out().size() == 3);
    pf.internal();
    CHECK(std::isinf(pf.advance()));
}

TEST_CASE("input_stream replays a sequence of single events", "[input_stream]") {
    auto piss = std::make_shared<std::istringstream>(
        "0 0 \n 1 1 \n 2 2 \n 3 3 \n 4 4 \n 5 5 \n 6 6 \n 7 7 \n 8 8 \n 9 9 \n 10 10");
    input_stream<Time, Message, int, int> pf{piss, Time(0)};
    CHECK(pf.advance() == Time(0));
    for (int i = 0; i < 10; i++) {
        REQUIRE(pf.out().size() == 1);
        CHECK(std::any_cast<int>(pf.out()[0]) == i);
        pf.internal();
        CHECK(pf.advance() == Time(1));
    }
    REQUIRE(pf.out().size() == 1);
    CHECK(std::any_cast<int>(pf.out()[0]) == 10);
    pf.internal();
    CHECK(std::isinf(pf.advance()));
}

TEST_CASE("input_stream replays pairs of events at each time step", "[input_stream]") {
    auto piss = std::make_shared<std::istringstream>(
        "1 1 \n 1 1 \n 2 2 \n 2 2 \n 3 3 \n 3 3 \n 4 4 \n 4 4 \n 5 5 \n 5 5");
    input_stream<Time, Message, int, int> pf{piss, Time(0)};
    CHECK(pf.advance() == Time(1));
    for (int i = 1; i < 5; i++) {
        REQUIRE(pf.out().size() == 2);
        CHECK(std::any_cast<int>(pf.out()[0]) == i);
        CHECK(std::any_cast<int>(pf.out()[1]) == i);
        pf.internal();
        CHECK(pf.advance() == Time(1));
    }
    REQUIRE(pf.out().size() == 2);
    CHECK(std::any_cast<int>(pf.out()[0]) == 5);
    CHECK(std::any_cast<int>(pf.out()[1]) == 5);
    pf.internal();
    CHECK(std::isinf(pf.advance()));
}

TEST_CASE("input_stream uses custom parser to read string messages", "[input_stream]") {
    auto piss = std::make_shared<std::istringstream>("1 hello \n 1 world \n 2 hello \n 2 world");
    input_stream<Time, Message, int, int> pf{
        piss, Time(0), [](const std::string &s, Time &t_next, std::any &m_next) {
            int tmp_int;
            std::string tmp_str;
            std::stringstream ss;
            ss.str(s);
            ss >> tmp_int;
            t_next = static_cast<Time>(tmp_int);
            ss >> tmp_str;
            m_next = static_cast<std::any>(tmp_str);
            std::string thrash;
            ss >> thrash;
            if (thrash.size() != 0)
                throw std::exception();
        }};
    CHECK(pf.advance() == Time(1));
    REQUIRE(pf.out().size() == 2);
    auto has = [&](const std::string &s) {
        auto out = pf.out();
        return std::any_of(out.begin(), out.end(),
                           [&](const std::any &m) { return std::any_cast<std::string>(m) == s; });
    };
    CHECK(has("hello"));
    CHECK(has("world"));
    pf.internal();
    CHECK(pf.advance() == Time(1));
    REQUIRE(pf.out().size() == 2);
    CHECK(has("hello"));
    CHECK(has("world"));
    pf.internal();
    CHECK(std::isinf(pf.advance()));
}
