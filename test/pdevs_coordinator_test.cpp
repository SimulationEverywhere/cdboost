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
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cdboost/convenience.hpp>
#include <cdboost/pdevs/basic_models/generator.hpp>
#include <cdboost/pdevs/basic_models/infinite_counter.hpp>
#include <cdboost/pdevs/basic_models/input_stream.hpp>
#include <cdboost/pdevs/basic_models/processor.hpp>
#include <cdboost/pdevs/coordinator.hpp>
#include <cdboost/pdevs/coupled.hpp>
#include <memory>
#include <sstream>

using namespace cdboost;
using namespace cdboost::pdevs;
using namespace cdboost::pdevs::basic_models;

using Time = double;

// Helper: processor whose confluence must not be called
template <class TIME, class MSG> class NoConfluenceProcessor : public processor<TIME, MSG> {
  public:
    explicit NoConfluenceProcessor(TIME t) : processor<TIME, MSG>(t) {}
    void confluence(const std::vector<MSG> &mb, const TIME &t) noexcept override {
        FAIL("confluence should not be called");
    }
};

// --- priority_queue_vector coordinator (template over MSG) ---

TEMPLATE_TEST_CASE("coordinator pqv: single generator produces right output", "[coordinator][pqv]",
                   std::any, int) {
    using MSG = TestType;
    auto pa   = std::make_shared<generator<Time, MSG>>(Time{1}, MSG{2});
    auto cm   = std::make_shared<coupled<Time, MSG>>(
        std::vector<std::shared_ptr<model<Time>>>{pa}, std::vector<std::shared_ptr<model<Time>>>{},
        std::vector<std::pair<std::shared_ptr<model<Time>>, std::shared_ptr<model<Time>>>>{},
        std::vector<std::shared_ptr<model<Time>>>{pa});
    auto c = std::make_shared<coordinator<Time, MSG, priority_queue_vector>>(cm);

    Time t = c->init(Time{0});
    CHECK(t == Time{1});
    auto reply = c->collectOutputs(t);
    c->advanceSimulation(Time{1});
    t = c->next();
    CHECK(t == Time(2));
    REQUIRE(reply.size() == 1);
    CHECK(std::any_cast<int>(reply[0]) == 2);
}

TEMPLATE_TEST_CASE("coordinator pqv: three cascaded generators produce correct output sequence",
                   "[coordinator][pqv]", std::any, int) {
    using MSG = TestType;
    auto pa1  = std::make_shared<generator<Time, MSG>>(Time{1}, MSG{1});
    auto cm1  = std::make_shared<coupled<Time, MSG>>(
        std::vector<std::shared_ptr<model<Time>>>{pa1}, std::vector<std::shared_ptr<model<Time>>>{},
        std::vector<std::pair<std::shared_ptr<model<Time>>, std::shared_ptr<model<Time>>>>{},
        std::vector<std::shared_ptr<model<Time>>>{pa1});
    auto pa2 = std::make_shared<generator<Time, MSG>>(Time{2}, MSG{2});
    auto cm2 = std::make_shared<coupled<Time, MSG>>(
        std::vector<std::shared_ptr<model<Time>>>{pa2, cm1},
        std::vector<std::shared_ptr<model<Time>>>{},
        std::vector<std::pair<std::shared_ptr<model<Time>>, std::shared_ptr<model<Time>>>>{},
        std::vector<std::shared_ptr<model<Time>>>{pa2, cm1});
    auto pa3 = std::make_shared<generator<Time, MSG>>(Time{3}, MSG{3});
    auto cm3 = std::make_shared<coupled<Time, MSG>>(
        std::vector<std::shared_ptr<model<Time>>>{pa3, cm2},
        std::vector<std::shared_ptr<model<Time>>>{},
        std::vector<std::pair<std::shared_ptr<model<Time>>, std::shared_ptr<model<Time>>>>{},
        std::vector<std::shared_ptr<model<Time>>>{pa3, cm2});
    auto c = std::make_shared<coordinator<Time, MSG, priority_queue_vector>>(cm3);

    Time t = c->init(Time{0});
    CHECK(t == Time{1});

    auto reply = c->collectOutputs(t);
    c->advanceSimulation(Time{1});
    t = c->next();
    CHECK(t == Time{2});
    REQUIRE(reply.size() == 1);
    CHECK(std::any_cast<int>(reply[0]) == 1);

    reply = c->collectOutputs(t);
    c->advanceSimulation(Time{2});
    t = c->next();
    CHECK(t == Time{3});
    REQUIRE(reply.size() == 2);
    CHECK(std::count_if(reply.begin(), reply.end(),
                        [](MSG &m) { return std::any_cast<int>(m) == 1; }) == 1);
    CHECK(std::count_if(reply.begin(), reply.end(),
                        [](MSG &m) { return std::any_cast<int>(m) == 2; }) == 1);
}

// --- nullqueue coordinator ---

TEST_CASE("coordinator nq: single generator produces right output", "[coordinator][nq]") {
    using MSG = std::any;
    auto pa   = std::make_shared<generator<Time, MSG>>(Time{1}, MSG{2});
    auto cm   = std::make_shared<coupled<Time, MSG>>(
        std::vector<std::shared_ptr<model<Time>>>{pa}, std::vector<std::shared_ptr<model<Time>>>{},
        std::vector<std::pair<std::shared_ptr<model<Time>>, std::shared_ptr<model<Time>>>>{},
        std::vector<std::shared_ptr<model<Time>>>{pa});
    auto c = std::make_shared<coordinator<Time, MSG, nullqueue>>(cm);

    Time t = c->init(Time{0});
    CHECK(t == Time{1});
    auto reply = c->collectOutputs(t);
    c->advanceSimulation(Time{1});
    t = c->next();
    CHECK(t == Time(2));
    REQUIRE(reply.size() == 1);
    CHECK(std::any_cast<int>(reply[0]) == 2);
}

TEST_CASE("coordinator nq: generator to infinite_counter with manual reset", "[coordinator][nq]") {
    using MSG = std::any;
    auto pg   = std::make_shared<generator<Time, MSG>>(Time{2}, MSG{1});
    auto pic  = std::make_shared<infinite_counter<Time, MSG>>();
    auto piss = std::make_shared<std::istringstream>(" 3 0 ");
    auto pf   = make_atomic_ptr<input_stream<Time, MSG, int, int>,
                                std::shared_ptr<std::istringstream>, Time>(piss, Time(0));
    auto cm   = std::make_shared<coupled<Time, MSG>>(
        std::vector<std::shared_ptr<model<Time>>>{pg, pic, pf},
        std::vector<std::shared_ptr<model<Time>>>{},
        std::vector<std::pair<std::shared_ptr<model<Time>>, std::shared_ptr<model<Time>>>>{
            {pg, pic}, {pf, pic}},
        std::vector<std::shared_ptr<model<Time>>>{pic});
    auto c = std::make_shared<coordinator<Time, MSG, nullqueue>>(cm);

    Time t = c->init(Time{0});
    CHECK(t == Time{2});

    auto reply = c->collectOutputs(t);
    c->advanceSimulation(Time{2});
    t = c->next();
    CHECK(t == Time{3});
    CHECK(reply.empty());

    reply = c->collectOutputs(t);
    c->advanceSimulation(Time{3});
    t = c->next();
    CHECK(t == Time{3});

    reply = c->collectOutputs(t);
    c->advanceSimulation(Time{3});
    t = c->next();
    CHECK(t == Time{4});
    REQUIRE(reply.size() == 1);
    CHECK(std::any_cast<int>(reply[0]) == 1);
}

TEST_CASE("coordinator nq: two generators to infinite_counter", "[coordinator][nq]") {
    using MSG = std::any;
    auto pg1  = std::make_shared<generator<Time, MSG>>(Time{1}, MSG{1});
    auto pg2  = std::make_shared<generator<Time, MSG>>(Time{2}, MSG{0});
    auto pic  = std::make_shared<infinite_counter<Time, MSG>>();
    auto cm   = std::make_shared<coupled<Time, MSG>>(
        std::vector<std::shared_ptr<model<Time>>>{pg1, pg2, pic},
        std::vector<std::shared_ptr<model<Time>>>{},
        std::vector<std::pair<std::shared_ptr<model<Time>>, std::shared_ptr<model<Time>>>>{
            {pg1, pic}, {pg2, pic}},
        std::vector<std::shared_ptr<model<Time>>>{pic});
    auto c = std::make_shared<coordinator<Time, MSG, nullqueue>>(cm);

    Time t = c->init(Time{0});
    CHECK(t == Time{1});

    auto reply = c->collectOutputs(t);
    c->advanceSimulation(Time{1});
    t = c->next();
    CHECK(t == Time{2});
    CHECK(reply.empty());

    reply = c->collectOutputs(t);
    c->advanceSimulation(Time{2});
    t = c->next();
    CHECK(t == Time{2});
    CHECK(reply.empty());

    reply = c->collectOutputs(t);
    c->advanceSimulation(Time{2});
    t = c->next();
    CHECK(t == Time{3});
    REQUIRE(reply.size() == 1);
    CHECK(std::any_cast<int>(reply[0]) == 2);
}

TEST_CASE("coordinator nq: confluence not called when generator output precedes processor",
          "[coordinator][nq]") {
    using MSG = std::any;
    auto pg   = std::make_shared<generator<Time, MSG>>(Time{2}, MSG{1});
    auto pp   = std::make_shared<processor<Time, MSG>>(Time{2});
    auto pt   = std::make_shared<NoConfluenceProcessor<Time, MSG>>(Time{2});
    auto cm   = std::make_shared<coupled<Time, MSG>>(
        std::vector<std::shared_ptr<model<Time>>>{pg, pp, pt},
        std::vector<std::shared_ptr<model<Time>>>{},
        std::vector<std::pair<std::shared_ptr<model<Time>>, std::shared_ptr<model<Time>>>>{
            {pg, pp}, {pg, pt}, {pp, pt}},
        std::vector<std::shared_ptr<model<Time>>>{pt});
    auto c = std::make_shared<coordinator<Time, MSG, nullqueue>>(cm);

    Time t = c->init(Time{0});
    CHECK(t == Time{2});
    auto reply = c->collectOutputs(t);
    c->advanceSimulation(Time{2});
    CHECK(reply.empty());
    // At t=4, pg fires again simultaneously with pp/pt — that is valid confluence,
    // not what this test covers. Just verify the next event is at t=4.
    CHECK(c->next() == Time{4});
}
