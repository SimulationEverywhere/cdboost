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

#include <cdboost/model.hpp>
#include <string>
#include <vector>

namespace cdboost {
    namespace pdevs {

        template <class TIME, class MSG> class atomic : public model<TIME> {
          public:
            using time_type    = TIME;
            using message_type = MSG;
            using model_type   = atomic<TIME, MSG>;

            atomic() noexcept : modelName("atomic") {}
            explicit atomic(const std::string &name) noexcept : modelName(name) {}

            virtual void internal() noexcept                                   = 0;
            virtual TIME advance() const noexcept                              = 0;
            virtual std::vector<MSG> out() const noexcept                      = 0;
            virtual void external(const std::vector<MSG> &mb, const TIME &t)   = 0;
            virtual void confluence(const std::vector<MSG> &mb, const TIME &t) = 0;

            const std::string as_string() const {
                return modelName;
            }
            virtual void print() noexcept {}

          private:
            std::string modelName;
        };

    } // namespace pdevs
} // namespace cdboost
