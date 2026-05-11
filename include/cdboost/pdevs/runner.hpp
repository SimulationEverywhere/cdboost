// SPDX-License-Identifier: BSD-2-Clause
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

#pragma once

#include <cdboost/log.hpp>
#include <cdboost/pdevs/coordinator.hpp>
#include <chrono>
#include <functional>
#include <memory>
#include <vector>

namespace cdboost {
    namespace pdevs {

        /**
         * @brief The Runner class runs the simulation.
         *
         * The runner is in charge of setting up the coordinators and simulators, the initial
         * conditions, the ending conditions and the loggers, then it runs the simulation and
         * displays the results.
         */
        template <class TIME, class MSG, template <class, class> class FEL = nullqueue>
        class runner {
            TIME _next;
            std::shared_ptr<coordinator<TIME, MSG, nullqueue>> _coordinator;
            std::function<std::string(const MSG &)> _msg_formatter; // empty = suppress tick output
            const TIME infinity;

            void process_output(const TIME &t, std::vector<MSG> &m) {
                if (!_msg_formatter)
                    return;
                for (auto &msg : m) {
                    cdboost::log::emit(cdboost::log::level::info, "tick", _msg_formatter(msg),
                                       cdboost::log::to_sim_double(t));
                }
            }

          public:
            /**
             * @brief Runner constructing from a coupled model with message output.
             * @param cm is the coupled model to simulate.
             * @param init_time is the initial time of the simulation.
             * @param msg_formatter converts a model output message to a string for the log msg
             *        field. Called once per message at each output-producing tick.
             */
            explicit runner(std::shared_ptr<coupled<TIME, MSG>> cm, const TIME &init_time,
                            std::function<std::string(const MSG &)> msg_formatter) noexcept
                : _msg_formatter(std::move(msg_formatter)), infinity(cm->infinity) {
                _coordinator = std::make_shared<coordinator<TIME, MSG, nullqueue>>(cm);
                _next        = _coordinator->init(init_time);
            }

            /**
             * @brief Runner constructing from a coupled model without message output.
             *        Tick events are suppressed; simulation_start/end/performance are still logged.
             * @param cm is the coupled model to simulate.
             * @param init_time is the initial time of the simulation.
             */
            explicit runner(std::shared_ptr<coupled<TIME, MSG>> cm, const TIME &init_time) noexcept
                : infinity(cm->infinity) {
                _coordinator = std::make_shared<coordinator<TIME, MSG, nullqueue>>(cm);
                _next        = _coordinator->init(init_time);
            }

            /**
             * @brief runUntil starts the simulation and stops when the next event is scheduled
             * after t.
             * @param t is the limit time for the simulation.
             * @return the TIME of the next event to happen when simulation stopped.
             */
            TIME runUntil(const TIME &t) noexcept {
                auto wall_start = std::chrono::steady_clock::now();
                cdboost::log::emit(cdboost::log::level::info, "simulation_start",
                                   "Starting simulation", cdboost::log::to_sim_double(_next));

                while (_next < t) {
                    auto out = _coordinator->collectOutputs(_next);
                    if (!out.empty())
                        process_output(_next, out);
                    _coordinator->advanceSimulation(_next);
                    _next = _coordinator->next();
                }

                cdboost::log::emit(cdboost::log::level::info, "simulation_end", "Simulation ended",
                                   cdboost::log::to_sim_double(t));

                auto wall_end = std::chrono::steady_clock::now();
                auto elapsed  = std::chrono::duration<double>(wall_end - wall_start).count();
                cdboost::log::emit(cdboost::log::level::info, "performance",
                                   std::format("Wall time: {} sec", elapsed));

                cdboost::log::flush();
                return _next;
            }

            /**
             * @brief runUntilPassivate starts the simulation and stops when there is no next
             * internal event to happen.
             */
            void runUntilPassivate() noexcept {
                auto wall_start = std::chrono::steady_clock::now();
                cdboost::log::emit(cdboost::log::level::info, "simulation_start",
                                   "Starting simulation until passivation",
                                   cdboost::log::to_sim_double(_next));

                while (_next != infinity) {
                    auto out = _coordinator->collectOutputs(_next);
                    if (!out.empty())
                        process_output(_next, out);
                    _coordinator->advanceSimulation(_next);
                    _next = _coordinator->next();
                }

                cdboost::log::emit(cdboost::log::level::info, "simulation_end",
                                   "Simulation passivated");

                auto wall_end = std::chrono::steady_clock::now();
                auto elapsed  = std::chrono::duration<double>(wall_end - wall_start).count();
                cdboost::log::emit(cdboost::log::level::info, "performance",
                                   std::format("Wall time: {} sec", elapsed));

                cdboost::log::flush();
            }
        };

    } // namespace pdevs
} // namespace cdboost
