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


#ifndef BOOST_SIMULATION_PDEVS_BM_INFINITE_COUNTER_H
#define BOOST_SIMULATION_PDEVS_BM_INFINITE_COUNTER_H
#include <boost/simulation/pdevs/atomic.hpp>
#include <boost/any.hpp>

namespace boost {
namespace simulation {
namespace pdevs {
namespace basic_models {

/**
 * @brief InfiniteCounter PDEVS Model.
 *
 * InfiniteCounter PDEVS Model:
 * - X = N
 * - Y = N
 * - S = {passive, active} x R+ x N
 * - internal(phase, t, count) = (passive, t, count)
 * - external(passive, t, count, e, x)=
 *           (passive, t-e, count+1) if x != 0
 *           (active, 0, 0) otherwise
 * - confluence(*) = external(active, 0, internal(phase, t, count))
 * - out (active, t, count) = count
 * - advance(phase, t, count) = t
*/

template<class TIME, class MSG>
class infinite_counter : public atomic<TIME, MSG>
{
    //state
    TIME _next;
    int _counter;
public:
    /**
     * @brief InfiniteCounter constructor.
     */
    explicit infinite_counter() noexcept : atomic<TIME, MSG>(), _counter(0){
        _next = atomic<TIME, MSG>::infinity;
    }
    /**
     * @brief internal function
     *
     * The internal function resets the counter.
     */
    void internal() noexcept {
        _next = atomic<TIME, MSG>::infinity;
        _counter = 0;
    }
    /**
     * @brief advance function.
     * @return Time until next internal event.
     */
    TIME advance() const noexcept { return _next; }
    /**
     * @brief out function.
     * @return _counter
     */
    std::vector<MSG> out() const noexcept { return std::vector<MSG>{_counter}; }
    /**
     * @brief external function.
     *
     * If the external function receives any 0 trigger the output setting the ta to 0.
     * The function also increments the counter for each received message
     * @param mb bag of messages.
     * @param t time the external input is received.
     */
    void external(const std::vector<MSG>& mb, const TIME& t) noexcept {
        int zeros = count_if(mb.begin(), mb.end(),
                        [](const MSG& m){
                            if(0 == boost::any_cast<int>(m)) return true;
                            else return false;
                        });
         if ( zeros ){
            _next = TIME{0};
            _counter += mb.size() - zeros;

        } else {
            _counter += mb.size();
        }
    }

    /**
     * @brief confluence function.
     * Execute the internal first, it means someone already requested a counting result before.
     *
     * @param msg
     * @param t time the external input is confluent with an internal transition.
     */
    void confluence(const std::vector<MSG>& mb, const TIME& t) noexcept {
        internal();
        external(mb, t);
    }

};

}
}
}
}

#endif // BOOST_SIMULATION_PDEVS_BM_INFINITE_COUNTER_H
