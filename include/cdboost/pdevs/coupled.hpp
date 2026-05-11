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

#include <algorithm>
#include <cdboost/pdevs/atomic.hpp>
#include <memory>
#include <ranges>
#include <utility>
#include <vector>

namespace cdboost {
    namespace pdevs {

        /**
         * @brief The coupled class represents PDEVS coupled models
         */
        template <class TIME, class MSG> class coupled : public model<TIME> {
          protected:
            /**
             * @brief The coupled_description struct provides the necesary data for Coordinators to
             * construct the hierarchy of simulation
             */
            struct coupled_description {
                std::vector<std::shared_ptr<model<TIME>>> models;
                std::vector<std::shared_ptr<model<TIME>>> external_input_coupling;
                std::vector<std::pair<std::shared_ptr<model<TIME>>, std::shared_ptr<model<TIME>>>>
                    internal_coupling; // first to second
                std::vector<std::shared_ptr<model<TIME>>> external_output_coupling;
            };

            coupled_description _desc;

          public:
            using time_type        = TIME;
            using message_type     = MSG;
            using model_type       = coupled<TIME, MSG>;
            using description_type = coupled_description;

            /**
             * @brief coupled receives the whole coupled model specs by pointers to models
             */
            coupled(std::initializer_list<std::shared_ptr<model<TIME>>> models,
                    std::initializer_list<std::shared_ptr<model<TIME>>> eic,
                    std::initializer_list<
                        std::pair<std::shared_ptr<model<TIME>>, std::shared_ptr<model<TIME>>>>
                        ic,
                    std::initializer_list<std::shared_ptr<model<TIME>>> eoc) {
                _desc.models                   = models;
                _desc.external_input_coupling  = eic;
                _desc.internal_coupling        = ic;
                _desc.external_output_coupling = eoc;
            }
            /**
             * @brief Coupled receives the whole coupled model spec by pointers to models
             * The difference with the other constructor is the use of vectors in place of
             * initilizer_lists for the case where the initializer_list can not be constructed
             * (because using dynamic construction or MS compiler).
             */
            coupled(
                std::vector<std::shared_ptr<model<TIME>>> models,
                std::vector<std::shared_ptr<model<TIME>>> eic,
                std::vector<std::pair<std::shared_ptr<model<TIME>>, std::shared_ptr<model<TIME>>>>
                    ic,
                std::vector<std::shared_ptr<model<TIME>>> eoc) {
                _desc.models                   = models;
                _desc.external_input_coupling  = eic;
                _desc.internal_coupling        = ic;
                _desc.external_output_coupling = eoc;
            }
            /**
             * @brief get_description provides the model in a way a coordinator
             * can read to construct the simulation hierarchy.
             */
            const coupled_description &get_description() noexcept {
                // this is a good place for an assert that whole model is properly constructed
                return _desc;
            }
        };

        /**
         * @brief The flattened_coupled class represents a coupled model PDEVS that has a single
         * level
         */
        template <class TIME, class MSG> class flattened_coupled : public coupled<TIME, MSG> {
          public:
            flattened_coupled(std::initializer_list<std::shared_ptr<model<TIME>>> models,
                              std::initializer_list<std::shared_ptr<model<TIME>>> eic,
                              std::initializer_list<std::pair<std::shared_ptr<model<TIME>>,
                                                              std::shared_ptr<model<TIME>>>>
                                  ic,
                              std::initializer_list<std::shared_ptr<model<TIME>>> eoc)
                : flattened_coupled(std::vector<std::shared_ptr<model<TIME>>>(models),
                                    std::vector<std::shared_ptr<model<TIME>>>(eic),
                                    std::vector<std::pair<std::shared_ptr<model<TIME>>,
                                                          std::shared_ptr<model<TIME>>>>(ic),
                                    std::vector<std::shared_ptr<model<TIME>>>(eoc)) {}

            flattened_coupled(
                std::vector<std::shared_ptr<model<TIME>>> models,
                std::vector<std::shared_ptr<model<TIME>>> eic,
                std::vector<std::pair<std::shared_ptr<model<TIME>>, std::shared_ptr<model<TIME>>>>
                    ic,
                std::vector<std::shared_ptr<model<TIME>>> eoc)
                : coupled<TIME, MSG>({}, {}, {}, {}) {
                for (auto &m : models) {
                    if (auto mc = std::dynamic_pointer_cast<coupled<TIME, MSG>>(m)) {
                        auto desc = mc->get_description();
                        std::ranges::copy(desc.models,
                                          std::back_inserter(coupled<TIME, MSG>::_desc.models));
                        std::ranges::copy(
                            desc.internal_coupling,
                            std::back_inserter(coupled<TIME, MSG>::_desc.internal_coupling));
                    } else {
                        coupled<TIME, MSG>::_desc.models.push_back(m);
                    }
                }
                for (auto &in : eic) {
                    if (auto mc = std::dynamic_pointer_cast<coupled<TIME, MSG>>(in)) {
                        std::ranges::copy(
                            mc->get_description().external_input_coupling,
                            std::back_inserter(coupled<TIME, MSG>::_desc.external_input_coupling));
                    } else {
                        coupled<TIME, MSG>::_desc.external_input_coupling.push_back(in);
                    }
                }
                for (auto &out : eoc) {
                    if (auto mc = std::dynamic_pointer_cast<coupled<TIME, MSG>>(out)) {
                        std::ranges::copy(
                            mc->get_description().external_output_coupling,
                            std::back_inserter(coupled<TIME, MSG>::_desc.external_output_coupling));
                    } else {
                        coupled<TIME, MSG>::_desc.external_output_coupling.push_back(out);
                    }
                }
                for (auto &[left, right] : ic) {
                    auto ml = std::dynamic_pointer_cast<coupled<TIME, MSG>>(left);
                    auto mr = std::dynamic_pointer_cast<coupled<TIME, MSG>>(right);
                    if (!ml && !mr) {
                        coupled<TIME, MSG>::_desc.internal_coupling.push_back({left, right});
                    } else if (!ml) {
                        for (auto &ri : mr->get_description().external_input_coupling)
                            coupled<TIME, MSG>::_desc.internal_coupling.push_back({left, ri});
                    } else if (!mr) {
                        for (auto &lo : ml->get_description().external_output_coupling)
                            coupled<TIME, MSG>::_desc.internal_coupling.push_back({lo, right});
                    } else {
                        for (auto &lo : ml->get_description().external_output_coupling)
                            for (auto &ri : mr->get_description().external_input_coupling)
                                coupled<TIME, MSG>::_desc.internal_coupling.push_back({lo, ri});
                    }
                }
            }
            /**
             * @brief get_description provides the model in a way a coordinator
             * can read to construct the simulation hierarchy.
             */
            const typename coupled<TIME, MSG>::coupled_description &get_description() noexcept {
                return coupled<TIME, MSG>::_desc;
            }
        };

    } // namespace pdevs
} // namespace cdboost
