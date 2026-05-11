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
#include <cdboost/cdboost.hpp>
#include <cdboost/pdevs/basic_models/processor.hpp>
#include <format>
#include <memory>
#include <sstream>
#include <string>

using namespace cdboost;
using namespace cdboost::pdevs;
using namespace cdboost::pdevs::basic_models;

// Simulation of an echo box doing 2 echoes of the input.

int main() {
    cdboost::log::init();

    cdboost::log::emit(cdboost::log::level::info, "model_setup",
                       "Creating atomic models for the 2 echos");

    auto echo1 = make_atomic_ptr<processor<double, std::any>, double>(double{1});
    auto echo2 = make_atomic_ptr<processor<double, std::any>, double>(double{3});

    cdboost::log::emit(cdboost::log::level::info, "model_setup",
                       "Coupling: input to echo1, echo1 to echo2, both to output");

    std::shared_ptr<coupled<double, std::any>> echobox(
        new coupled<double, std::any>{{echo1, echo2}, {echo1}, {{echo1, echo2}}, {echo1, echo2}});

    cdboost::log::emit(cdboost::log::level::info, "model_setup", "Creating input stream model");

    auto piss = std::make_shared<std::istringstream>();
    piss->str("1 1 \n 4 4 \n 5 5 \n 6 6 \n 8 8 \n 9 9 ");
    auto pf = make_atomic_ptr<input_stream<double, std::any, int, int>,
                              std::shared_ptr<std::istringstream>, double>(piss, double{0});

    cdboost::log::emit(cdboost::log::level::info, "model_setup",
                       "Coupling echobox to input stream");

    std::shared_ptr<coupled<double, std::any>> root(
        new coupled<double, std::any>{{pf, echobox}, {}, {{pf, echobox}}, {echobox}});

    double initial_time{0};
    runner<double, std::any> r(root, initial_time, [](const std::any &m) -> std::string {
        return std::format("{}", std::any_cast<int>(m));
    });

    r.runUntilPassivate();
    return 0;
}
