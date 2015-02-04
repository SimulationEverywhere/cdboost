/**
 * Copyright (c) 2013-2015, Damian Vicino
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


#ifndef BOOST_SIMULATION_PDEVS_BM_GENERATOR_H
#define BOOST_SIMULATION_PDEVS_BM_GENERATOR_H
#include <boost/simulation/pdevs/atomic.hpp>

namespace boost {
namespace simulation {
namespace pdevs {
namespace basic_models {
/**
 * @brief Generator PDEVS Model
 *
 * Generator PDEVS Model(period, outvalue):
 * - X = {}
 * - Y = {outvalue}
 * - S = {passive, active} x Multiples(period)
 * - internal(phase, t) = ("active", period)
 * - external = {}
 * - out ("active", t) = outvalue
 * - advance(phase, t) = period - t
*/
template<class TIME, class MSG>
class generator : public atomic<TIME, MSG>
{
    TIME _period;
    std::vector<MSG> _outvalue;
public:
    /**
     * @brief Generator constructor.
     *
     * @param period Amount of time between ticks.
     * @param outvalue Value to be returned by out function.
     */
    explicit generator(TIME period, MSG outvalue=1) noexcept : _period(period), _outvalue(std::vector<MSG>{outvalue}) {}
    /**
     * @brief internal function.
     */
    void internal() noexcept {}
    /**
     * @brief advance function.
     * @return Time until next internal event.
     */
    TIME advance() const noexcept { return _period; }
    /**
     * @brief out function.
     * @return MSG defined in contruction.
     */
    std::vector<MSG> out() const noexcept { return _outvalue; }
    /**
     * @brief external function domain is empty, so it throws.
     * @param msg external input message.
     * @param t time the external input is received.
     */
    void external(const std::vector<MSG>& mb, const TIME& t) noexcept { assert(false && "No external input is expected by this model"); }

    /**
     * @brief confluence function.
     * Execute the internal first, it means someone already requested a counting result before.
     *
     * @param msg
     * @param t time the external input is confluent with an internal transition.
     */
    void confluence(const std::vector<MSG>& mb, const TIME& t)  noexcept  { assert(false && "No external input is expected by this model"); }

};

}
}
}
}
#endif // BOOST_SIMULATION_PDEVS_BM_GENERATOR_H
