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

#include <algorithm>
#include <any>
#include <catch2/catch_test_macros.hpp>
#include <cdboost/convenience.hpp>
#include <cdboost/pdevs/basic_models/generator.hpp>
#include <cdboost/pdevs/basic_models/infinite_counter.hpp>
#include <cdboost/pdevs/coupled.hpp>
#include <cdboost/rational_time.hpp>
#include <memory>
#include <utility>
#include <vector>

using namespace cdboost;
using namespace cdboost::pdevs;
using namespace cdboost::pdevs::basic_models;

using Time    = boost::rational<int>;
using Message = std::any;

TEST_CASE("coupled with single generator via initializer_list", "[coupled]") {
    auto pg = make_atomic_ptr<generator<Time, Message>, Time>(Time{1});
    coupled<Time, Message> pc{{pg}, {}, {}, {pg}};
    auto desc = pc.get_description();
    CHECK(desc.models.size() == 1);
    auto m = std::dynamic_pointer_cast<atomic<Time, Message>>(desc.models[0]);
    CHECK(m == pg);
    CHECK(desc.external_input_coupling.empty());
    CHECK(desc.internal_coupling.empty());
    REQUIRE(desc.external_output_coupling.size() == 1);
    CHECK(std::dynamic_pointer_cast<atomic<Time, Message>>(desc.external_output_coupling[0]) == pg);
}

TEST_CASE("coupled with generator to infinite_counter via initializer_list", "[coupled]") {
    auto pg  = make_atomic_ptr<generator<Time, Message>, Time>(Time{1});
    auto pic = make_atomic_ptr<infinite_counter<Time, Message>>();
    coupled<Time, Message> pc{{pg, pic}, {pic}, {{pg, pic}}, {pic}};
    auto desc = pc.get_description();
    CHECK(desc.models.size() == 2);
    CHECK(desc.external_input_coupling.size() == 1);
    CHECK(desc.internal_coupling.size() == 1);
    CHECK(desc.external_output_coupling.size() == 1);
    CHECK(desc.internal_coupling[0].first == pg);
    CHECK(desc.internal_coupling[0].second == pic);
}

TEST_CASE("coupled with nested coupled models via initializer_list", "[coupled]") {
    auto pg1 = make_atomic_ptr<generator<Time, Message>, Time>(Time{1});
    auto pg2 = make_atomic_ptr<generator<Time, Message>, Time>(Time{1});
    auto pic = make_atomic_ptr<infinite_counter<Time, Message>>();
    auto pc1 = std::make_shared<coupled<Time, Message>>(
        std::vector<std::shared_ptr<model<Time>>>{pg1, pic},
        std::vector<std::shared_ptr<model<Time>>>{pic},
        std::vector<std::pair<std::shared_ptr<model<Time>>, std::shared_ptr<model<Time>>>>{
            {pg1, pic}},
        std::vector<std::shared_ptr<model<Time>>>{pic});
    coupled<Time, Message> pc2{{pg2, pc1}, {pc1}, {{pg2, pc1}}, {pc1}};
    auto desc = pc2.get_description();
    CHECK(desc.models.size() == 2);
    CHECK(desc.external_input_coupling.size() == 1);
    CHECK(desc.internal_coupling.size() == 1);
    CHECK(desc.external_output_coupling.size() == 1);
}

TEST_CASE("coupled with single generator via vector constructors", "[coupled]") {
    auto pg = make_atomic_ptr<generator<Time, Message>, Time>(Time{1});
    coupled<Time, Message> pc{
        std::vector<std::shared_ptr<model<Time>>>{pg}, std::vector<std::shared_ptr<model<Time>>>{},
        std::vector<std::pair<std::shared_ptr<model<Time>>, std::shared_ptr<model<Time>>>>{},
        std::vector<std::shared_ptr<model<Time>>>{pg}};
    auto desc = pc.get_description();
    CHECK(desc.models.size() == 1);
    CHECK(desc.models[0] == pg);
    CHECK(desc.external_output_coupling[0] == pg);
}
