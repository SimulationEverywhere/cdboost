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

#include <cdboost/cdboost.hpp>
#include <cdboost/pdevs/basic_models/generator.hpp>
#include <cdboost/rational_time.hpp>
#include <memory>
#include <string>

using namespace cdboost;
using namespace cdboost::pdevs;
using namespace cdboost::pdevs::basic_models;

// Simulation of a clock with 3 needles (hour, minute, second) using rational time.

int main() {
    cdboost::log::init();

    cdboost::log::emit(cdboost::log::level::info, "model_setup",
                       "Creating atomic models for 3 needles");

    auto second =
        make_atomic_ptr<generator<boost::rational<int>, std::string>, boost::rational<int>,
                        std::string>(boost::rational<int>{1}, std::string("second"));
    auto minute =
        make_atomic_ptr<generator<boost::rational<int>, std::string>, boost::rational<int>,
                        std::string>(boost::rational<int>{60}, std::string("minute"));
    auto hour = make_atomic_ptr<generator<boost::rational<int>, std::string>, boost::rational<int>,
                                std::string>(boost::rational<int>{3600}, std::string("hour"));

    cdboost::log::emit(cdboost::log::level::info, "model_setup",
                       "Coupling the 3 needles into a clock model");

    std::shared_ptr<coupled<boost::rational<int>, std::string>> clock(
        new coupled<boost::rational<int>, std::string>{
            {second, minute, hour}, {}, {}, {second, minute, hour}});

    boost::rational<int> initial_time{0};
    boost::rational<int> end_time{7200}; // 2 hours

    cdboost::log::emit(cdboost::log::level::info, "model_setup",
                       std::format("Starting simulation until time: 7200/1 seconds"));

    runner<boost::rational<int>, std::string> r(clock, initial_time,
                                                [](const std::string &m) { return m; });

    r.runUntil(end_time);
    return 0;
}
