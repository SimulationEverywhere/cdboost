/**
 * Copyright (c) 2013-2015, Damian Vicino & Daniella Niyonkuru
 * Modified by Daniella Niyonkuru (21/7/15 -> Port addition)
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


#ifndef BOOST_SIMULATION_CONVENIENCE_H
#define BOOST_SIMULATION_CONVENIENCE_H
namespace boost {
namespace simulation {

/**
 * @brief create a pointer to a new atomic model of kind MODEL with its constructor parameters
 * @template MODEL model to be constructed
 * @template Args parameters to perfect forward to constructor of model
 * @param args parameters to perfect forward to constructor
 * @return a shared pointer to the atomic model constructed
 */

//create a shared pointer to a pdevs::atomic model
template<class MODEL, typename... Args>
std::shared_ptr<pdevs::atomic<typename MODEL::time_type, typename MODEL::message_type>> make_atomic_ptr(Args... args) noexcept {
    return std::make_shared<MODEL>(std::forward<Args>(args)...);
}

//create a shared pointer to a hardware port
template<class MODEL, typename... Args>
std::shared_ptr<pdevs::port<typename MODEL::time_type, typename MODEL::message_type>> make_port_ptr(Args... args) noexcept {
    return std::make_shared<MODEL>(std::forward<Args>(args)...);
}

}
}

#endif // BOOST_SIMULATION_CONVENIENCE_H
