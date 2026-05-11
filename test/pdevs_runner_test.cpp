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
#include <cdboost/pdevs/basic_models/generator.hpp>
#include <cdboost/pdevs/basic_models/input_stream.hpp>
#include <cdboost/pdevs/runner.hpp>
#include <cmath>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

using namespace cdboost::pdevs;
using namespace cdboost::pdevs::basic_models;

using Time    = double;
using Message = std::any;

TEST_CASE("runner runs generator until time 10 (silent)", "[runner]") {
    auto pa = std::make_shared<generator<Time, Message>>(Time{1});
    auto cm = std::make_shared<coupled<Time, Message>>(
        std::vector<std::shared_ptr<cdboost::model<Time>>>{pa},
        std::vector<std::shared_ptr<cdboost::model<Time>>>{},
        std::vector<std::pair<std::shared_ptr<cdboost::model<Time>>,
                              std::shared_ptr<cdboost::model<Time>>>>{},
        std::vector<std::shared_ptr<cdboost::model<Time>>>{pa});
    runner<Time, Message> r(cm, Time{0});
    CHECK(r.runUntil(Time{10}) == Time{10});
}

TEST_CASE("runner stops when model goes passive", "[runner]") {
    auto piss = std::make_shared<std::istringstream>("1 1 \n 4 4 \n 5 5 \n 6 6 \n 8 8 \n 9 9 ");
    auto pf   = std::make_shared<input_stream<Time, Message, int, int>>(piss, Time{0});
    auto cm   = std::make_shared<coupled<Time, Message>>(
        std::vector<std::shared_ptr<cdboost::model<Time>>>{pf},
        std::vector<std::shared_ptr<cdboost::model<Time>>>{},
        std::vector<std::pair<std::shared_ptr<cdboost::model<Time>>,
                                std::shared_ptr<cdboost::model<Time>>>>{},
        std::vector<std::shared_ptr<cdboost::model<Time>>>{pf});
    runner<Time, Message> r(cm, Time{0});
    CHECK(std::isinf(r.runUntil(Time{20})));
}

TEST_CASE("runner stops mid-sequence and returns next event time", "[runner]") {
    auto piss = std::make_shared<std::istringstream>("1 1 \n 4 4 \n 5 5 \n 6 6 \n 8 8 \n 9 9 ");
    auto pf   = std::make_shared<input_stream<Time, Message, int, int>>(piss, Time{0});
    auto cm   = std::make_shared<coupled<Time, Message>>(
        std::vector<std::shared_ptr<cdboost::model<Time>>>{pf},
        std::vector<std::shared_ptr<cdboost::model<Time>>>{},
        std::vector<std::pair<std::shared_ptr<cdboost::model<Time>>,
                                std::shared_ptr<cdboost::model<Time>>>>{},
        std::vector<std::shared_ptr<cdboost::model<Time>>>{pf});
    runner<Time, Message> r(cm, Time{0});
    CHECK(r.runUntil(Time{7}) == Time{8});
}

TEST_CASE("runner runUntilPassivate completes without throwing", "[runner]") {
    auto piss = std::make_shared<std::istringstream>("1 1 \n 4 4 \n 5 5 \n 6 6 \n 8 8 \n 9 9 ");
    auto pf   = std::make_shared<input_stream<Time, Message, int, int>>(piss, Time{0});
    auto cm   = std::make_shared<coupled<Time, Message>>(
        std::vector<std::shared_ptr<cdboost::model<Time>>>{pf},
        std::vector<std::shared_ptr<cdboost::model<Time>>>{},
        std::vector<std::pair<std::shared_ptr<cdboost::model<Time>>,
                                std::shared_ptr<cdboost::model<Time>>>>{},
        std::vector<std::shared_ptr<cdboost::model<Time>>>{pf});
    runner<Time, Message> r(cm, Time{0});
    CHECK_NOTHROW(r.runUntilPassivate());
}

TEST_CASE("runner with formatter: generator formatter called 9 times with value 1", "[runner]") {
    auto pa = std::make_shared<generator<Time, Message>>(Time{1});
    auto cm = std::make_shared<coupled<Time, Message>>(
        std::vector<std::shared_ptr<cdboost::model<Time>>>{pa},
        std::vector<std::shared_ptr<cdboost::model<Time>>>{},
        std::vector<std::pair<std::shared_ptr<cdboost::model<Time>>,
                              std::shared_ptr<cdboost::model<Time>>>>{},
        std::vector<std::shared_ptr<cdboost::model<Time>>>{pa});
    std::vector<std::string> captured;
    runner<Time, Message> r(cm, Time{0}, [&](const std::any &m) -> std::string {
        auto s = std::to_string(std::any_cast<int>(m));
        captured.push_back(s);
        return s;
    });
    r.runUntil(Time{10});
    REQUIRE(captured.size() == 9);
    CHECK(std::all_of(captured.begin(), captured.end(), [](const auto &s) { return s == "1"; }));
}

TEST_CASE("runner with formatter: input_stream formatter called for each event", "[runner]") {
    auto piss = std::make_shared<std::istringstream>("1 1 \n 4 4 \n 5 5 \n 6 6 \n 8 8 \n 9 9 ");
    auto pf   = std::make_shared<input_stream<Time, Message, int, int>>(piss, Time{0});
    auto cm   = std::make_shared<coupled<Time, Message>>(
        std::vector<std::shared_ptr<cdboost::model<Time>>>{pf},
        std::vector<std::shared_ptr<cdboost::model<Time>>>{},
        std::vector<std::pair<std::shared_ptr<cdboost::model<Time>>,
                                std::shared_ptr<cdboost::model<Time>>>>{},
        std::vector<std::shared_ptr<cdboost::model<Time>>>{pf});
    std::vector<std::string> captured;
    runner<Time, Message> r(cm, Time{0}, [&](const std::any &m) -> std::string {
        auto s = std::to_string(std::any_cast<int>(m));
        captured.push_back(s);
        return s;
    });
    r.runUntilPassivate();
    REQUIRE(captured.size() == 6);
    CHECK(captured[0] == "1");
    CHECK(captured[1] == "4");
    CHECK(captured[2] == "5");
    CHECK(captured[3] == "6");
    CHECK(captured[4] == "8");
    CHECK(captured[5] == "9");
}
