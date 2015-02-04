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


#ifndef BOOST_SIMULATION_PDEVS_PROCESSOR_H
#define BOOST_SIMULATION_PDEVS_PROCESSOR_H
#include <queue>
#include <boost/simulation/pdevs/atomic.hpp>

namespace boost {
namespace simulation {
namespace pdevs {
namespace basic_models {
/**
 * @brief Processor PDEVS Model.
 *
 * Processor PDEVS Model:
 * - X = R
 * - Y = R
 * - S = {passive, active} x R+ x R
 * - internal(phase, t, job) = (passive, processing_time, job)
 * - external(phase, t, job, e, x)=
 *           ( active, processing_time, job) if phase=passive
 *           ( phase, t-e, job) otherwise
 * - confluence -> internal; external
 * - out (active, t, job) = job
 * - advance(phase, t, job) = t
*/
template<class TIME, class MSG>
class processor : public atomic<TIME, MSG>
{
    TIME _next;
    std::queue<MSG> _jobs;
    TIME _processing;
public:
    /**
     * @brief Processor constructor.
     */
    explicit processor(TIME processing) noexcept : _next(atomic<TIME, MSG>::infinity), _processing(processing){}
    /**
     * @brief internal function.
     *
     * The internal function removes the first job in the list and adjusts the time in _next
     */
    void internal() noexcept {
        _jobs.pop();
        _next = (0 == _jobs.size()?atomic<TIME, MSG>::infinity:_processing);
    }
    /**
     * @brief advance function.
     * @return Time until next internal event.
     */
    TIME advance() const noexcept {return _next;}
    /**
     * @brief out function.
     * @return first job
     */
    std::vector<MSG> out() const noexcept { return {_jobs.front()}; }
    /**
     * @brief external function.
     *
     * The external function receives jobs which are output after being processed.
     * All jobs have the same processing time, and only 1 can be processed at the time.
     * @param mb receives a bag of jobs.
     * @param t time the external input is received (relative to last advace).
     */
     void external(const std::vector<MSG>& mb, const TIME& t) noexcept {
        _next = (0 == _jobs.size()?_processing: (_next-t));
        for (auto& m :mb){
            _jobs.push(m);
        }
    }
     /**
     * @brief confluence function as processing internal first and external inmediately afterward.
     *
     * @param mb receives a bag of jobs.
     * @param t time the external input is received (relative to last advace).
     */
    virtual void confluence(const std::vector<MSG>& mb, const TIME& t) noexcept {
        internal();
        external(mb, TIME(0));
    }

};

}
}
}
}

#endif // BOOST_SIMULATION_PDEVS_PROCESSOR_H
