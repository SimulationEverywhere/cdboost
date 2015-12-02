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


#ifndef BOOST_SIMULATION_PDEVS_ATOMIC_H
#define BOOST_SIMULATION_PDEVS_ATOMIC_H
#include <vector>
#include <boost/simulation/model.hpp>

namespace boost {
namespace simulation {
namespace pdevs {

/**
 * @brief The pdevs::atomic class is the base for all PDEVS atomic models.
 *
 * The atomic class has the basic 4 functions that need to be implemented by a model to be used.
 * The Simulator objects instantiate the atomic models and call the operations in the proper order
 * to  produce the simulation.
 */

template <class TIME, class MSG>
class atomic : public model<TIME>
{
public:
    using time_type=TIME;
    using message_type=MSG; //Message suggested for most simulations is boost::any
    using model_type=atomic<TIME, MSG>;

    atomic() noexcept : modelName("atomic") {}

    atomic(const std::string &name) noexcept : modelName( name ) {}
    /**
     * @brief internal transition function as defined in PDEVS
     */
    virtual void internal() noexcept  = 0;
    /**
     * @brief time advance function as defined in PDEVS
     * @return Time until next internal event
     */
    virtual TIME advance() const noexcept = 0;
    /**
     * @brief output function as defined in PDEVS
     * @return output message
     */
    virtual std::vector<MSG> out() const noexcept = 0;
    /**
     * @brief external function as defined in PDEVS
     *
     * @param mb is a bag of messages coming from outside.
     * @param t is the time the message is received
     */
    virtual void external(const std::vector<MSG>& mb, const TIME& t) noexcept = 0;
    /**
     * @brief confluence function as defined in PDEVS
     *
     * @param mb is a bag of messages coming from outside
     * @param t is the time the message is received
     */
    virtual void confluence(const std::vector<MSG>& mb, const TIME& t) noexcept = 0;
    /**
     * @brief asString returns the name of the port
     */
	const std::string asString() const { return modelName; }
    /**
     * @brief print prints the name of the port - To be implemented by the user
     */
	virtual void print() noexcept = 0;

private:
    std::string modelName;
};

}
}
}

#endif // BOOST_SIMULATION_PDEVS_ATOMIC_H



