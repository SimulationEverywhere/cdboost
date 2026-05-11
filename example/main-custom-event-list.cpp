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
#include <cdboost/pdevs/basic_models/generator.hpp>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace cdboost;
using namespace cdboost::pdevs;
using namespace cdboost::pdevs::basic_models;

// Demonstrates processing a custom event list where messages are strings parsed from a stream.

int main() {
    cdboost::log::init();

    cdboost::log::emit(cdboost::log::level::info, "model_setup",
                       "Creating input stream for custom event list");

    std::shared_ptr<std::istringstream> piss{new std::istringstream{}};
    piss->str("1 hello \n 1 world \n 2 hello \n 2 world");

    cdboost::log::emit(cdboost::log::level::info, "model_setup",
                       "Creating pdevs istream model with string messages");

    auto pf = make_atomic_ptr<input_stream<double, std::any, int, std::string>,
                              std::shared_ptr<std::istringstream>, double>(
        piss, double(0), [](const std::string &s, double &t_next, std::any &m_next) {
            int tmp_next;
            std::string tmp_next_out;
            std::stringstream ss;
            ss.str(s);
            ss >> tmp_next;
            t_next = static_cast<double>(tmp_next);
            ss >> tmp_next_out;
            m_next = static_cast<std::any>(tmp_next_out);
            std::string thrash;
            ss >> thrash;
            if (!thrash.empty())
                throw std::runtime_error("unexpected token in event stream");
        });

    cdboost::log::emit(cdboost::log::level::info, "model_setup",
                       "Coupling model and running until all events are consumed");

    std::shared_ptr<coupled<double, std::any>> player(
        new coupled<double, std::any>{{pf}, {}, {}, {pf}});

    double initial_time{0};
    runner<double, std::any> r(player, initial_time, [](const std::any &m) -> std::string {
        return std::any_cast<std::string>(m);
    });

    r.runUntilPassivate();
    return 0;
}
