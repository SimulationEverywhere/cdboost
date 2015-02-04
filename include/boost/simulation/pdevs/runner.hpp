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

#ifndef BOOST_SIMULATION_PDEVS_RUNNER_H
#define BOOST_SIMULATION_PDEVS_RUNNER_H

#include <iostream>
#include <boost/simulation/pdevs/coordinator.hpp>
#include <boost/any.hpp>

namespace boost {
namespace simulation {
namespace pdevs {

/**
 * @brief The Runner class runs the simulation.
 *
 * The runner is in charge of setting up the coordinators and simulators, the initial
 * conditions, the ending conditions and the loggers, then it runs the simulation and
 * displays the results.
 */
template <class TIME, class MSG, template<class, class> class FEL=nullqueue>
class runner
{
    TIME _next; //next scheduled event
    std::shared_ptr<coordinator<TIME, MSG, nullqueue>> _coordinator; //ecoordinator of the top level coupled model.
    bool _silent;
    std::ostream& _out_stream;
    void (*_out_interpreter)(std::ostream&, MSG);

    void process_output(TIME t, std::vector<MSG>& m) noexcept {
        for ( auto& msg : m){
            _out_stream << t << " ";
            _out_interpreter(_out_stream, msg);
            _out_stream << std::endl;
        }
    }

    const TIME infinity;


public:
    //contructors
    /**
     * @brief Runner constructing from a M model connected to an output.
     * @param cm is the coupled model to simulate.
     * @param init_time is the initial time of the simulation.
     * @param out_stream is where the model output goes for displaying.
     * @param out_interpreter a function to handle the insertion of
     *        model output messages into the out_stream.
     */
    explicit runner(std::shared_ptr<coupled<TIME, MSG>> cm,
                    const TIME& init_time, std::ostream& out_stream,
                    decltype(_out_interpreter) out_interpreter) noexcept
        : _out_stream(out_stream), _out_interpreter(out_interpreter), infinity(cm->infinity)
    {
        _coordinator.reset(new coordinator<TIME, MSG, nullqueue>{cm});
        _next = _coordinator->init(init_time);
        _silent = false;
    }


    /**
     * @brief Runner constructing from a M model, its silent, no output.
     * @param cm is the coupled model in Extended DEVS to simulate.
     * @param init_time is the initial time of the simulation.
     */
    explicit runner(std::shared_ptr<coupled<TIME, MSG>> cm, const TIME& init_time) noexcept
     : _out_stream( std::cerr ), //for debuging purposes
      infinity(cm->infinity)
    {
        _coordinator.reset(new coordinator<TIME, MSG, nullqueue>{cm});
        _next = _coordinator->init(init_time);
        _silent = true;
    }

    /**
     * @brief runUntil starts the simulation and stops when the next event is scheduled after t.
     * @param t is the limit time for the simulation.
     * @return the TIME of the next event to happen when simulation stopped.
     */
    TIME runUntil(const TIME& t) noexcept
    {
        if (_silent){
            while (_next < t)
            {
                _coordinator->advanceSimulation( _next);
                _next = _coordinator->next();
            }

        } else {
            while (_next < t)
            {
                auto out = _coordinator->collectOutputs(_next);
                if (!out.empty()) process_output(_next, out);

                _coordinator->advanceSimulation( _next);
                _next = _coordinator->next();
            }
        }
        return _next;
    }

    /**
     * @brief runUntilPassivate starts the simulation and stops when there is no next internal event to happen.
     */
    void runUntilPassivate() noexcept
    {
        if (_silent){
            while ( _next !=  infinity )
            {
                _coordinator->advanceSimulation( _next);
                _next = _coordinator->next();
            }
        } else {
            while ( _next != infinity)
            {
                auto out = _coordinator->collectOutputs(_next);
                if (!out.empty()) process_output(_next, out);

                _coordinator->advanceSimulation( _next);
                _next = _coordinator->next();
            }
        }
    }
};


}
}
}


#endif // BOOST_SIMULATION_PDEVS_RUNNER_H
