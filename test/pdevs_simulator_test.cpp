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
#include <cdboost/convenience.hpp>
#include <cdboost/pdevs/basic_models/generator.hpp>
#include <cdboost/pdevs/basic_models/infinite_counter.hpp>
#include <cdboost/pdevs/basic_models/input_stream.hpp>
#include <cdboost/pdevs/coordinator.hpp>
#include <cmath>
#include <memory>
#include <sstream>

using namespace cdboost;
using namespace cdboost::pdevs;
using namespace cdboost::pdevs::basic_models;

using Time    = double;
using Message = std::any;

TEST_CASE("simulator: generator init returns period", "[simulator][generator]") {
    auto pa = make_atomic_ptr<generator<Time, Message>, Time>(Time{1});
    coordinator<Time, Message> s{pa};
    CHECK(s.init(Time(0)) == Time{1});
}

TEST_CASE("simulator: generator init with offset", "[simulator][generator]") {
    auto pa = make_atomic_ptr<generator<Time, Message>, Time>(Time{1});
    coordinator<Time, Message> s{pa};
    CHECK(s.init(Time(3)) == Time{4});
}

TEST_CASE("simulator: generator in coupled init returns period", "[simulator][generator]") {
    auto pa = make_atomic_ptr<generator<Time, Message>, Time>(Time{1});
    auto c  = std::make_shared<coupled<Time, Message>>(
        std::vector<std::shared_ptr<model<Time>>>{pa}, std::vector<std::shared_ptr<model<Time>>>{},
        std::vector<std::pair<std::shared_ptr<model<Time>>, std::shared_ptr<model<Time>>>>{},
        std::vector<std::shared_ptr<model<Time>>>{pa});
    coordinator<Time, Message> s{c};
    CHECK(s.init(Time(0)) == Time{1});
}

TEST_CASE("simulator: infinite_counter init returns infinity", "[simulator][infinite_counter]") {
    auto pa = make_atomic_ptr<infinite_counter<Time, Message>>();
    coordinator<Time, Message> s{pa};
    CHECK(std::isinf(s.init(Time(0))));
}

TEST_CASE("simulator: infinite_counter receives stream then outputs count",
          "[simulator][infinite_counter]") {
    auto pa   = make_atomic_ptr<infinite_counter<Time, Message>>();
    auto piss = std::make_shared<std::istringstream>(
        "1 1 \n 1 2 \n 1 3 \n 1 4 \n 2 5 \n 2 6 \n 2 7 \n 2 8 \n 2 0 ");
    auto pf = make_atomic_ptr<input_stream<Time, Message, int, int>>(piss, Time(0));
    auto pc = std::make_shared<coupled<Time, Message>>(
        std::vector<std::shared_ptr<model<Time>>>{pf, pa},
        std::vector<std::shared_ptr<model<Time>>>{},
        std::vector<std::pair<std::shared_ptr<model<Time>>, std::shared_ptr<model<Time>>>>{
            {pf, pa}},
        std::vector<std::shared_ptr<model<Time>>>{pa});
    coordinator<Time, Message> s{pc};

    Time t = s.init(Time{0});
    CHECK(t == Time{1});
    auto reply = s.collectOutputs(t);
    s.advanceSimulation(Time{1});
    CHECK(reply.empty());
    CHECK(s.next() == Time{2});

    reply = s.collectOutputs(t);
    s.advanceSimulation(Time{2});
    CHECK(reply.empty());
    CHECK(s.next() == Time{2});

    reply = s.collectOutputs(Time{2});
    s.advanceSimulation(Time{2});
    REQUIRE(reply.size() == 1);
    CHECK(std::any_cast<int>(reply[0]) == 8);
    CHECK(std::isinf(s.next()));
}

TEST_CASE("simulator: infinite_counter confluence at time 1", "[simulator][infinite_counter]") {
    auto pa   = make_atomic_ptr<infinite_counter<Time, Message>>();
    auto piss = std::make_shared<std::istringstream>(
        "1 1 \n 1 2 \n 1 3 \n 1 4 \n 1 0 \n 2 5 \n 2 6 \n 2 7 \n 2 8 \n 2 0 ");
    auto pf = make_atomic_ptr<input_stream<Time, Message, int, int>,
                              std::shared_ptr<std::istringstream>, Time>(piss, Time(0));
    auto pc = std::make_shared<coupled<Time, Message>>(
        std::vector<std::shared_ptr<model<Time>>>{pf, pa},
        std::vector<std::shared_ptr<model<Time>>>{},
        std::vector<std::pair<std::shared_ptr<model<Time>>, std::shared_ptr<model<Time>>>>{
            {pf, pa}},
        std::vector<std::shared_ptr<model<Time>>>{pa});
    coordinator<Time, Message> s{pc};

    Time t = s.init(Time{0});
    CHECK(t == Time{1});

    auto reply = s.collectOutputs(t);
    CHECK(reply.empty());
    CHECK(s.next() == Time{1});

    s.advanceSimulation(Time{1});
    CHECK(s.next() == Time{1});

    reply = s.collectOutputs(Time{1});
    REQUIRE(reply.size() == 1);
    CHECK(std::any_cast<int>(reply[0]) == 4);

    s.advanceSimulation(Time{1});
    CHECK(s.next() == Time{2});

    reply = s.collectOutputs(Time{2});
    CHECK(reply.empty());

    s.advanceSimulation(Time{2});
    CHECK(s.next() == Time{2});

    reply = s.collectOutputs(Time{2});
    REQUIRE(reply.size() == 1);
    CHECK(std::any_cast<int>(reply[0]) == 4);

    s.advanceSimulation(Time{2});
    CHECK(std::isinf(s.next()));
}
