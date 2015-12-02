/**
 * Copyright (c) 2013-2015, Damian Vicino & Daniella Niyonkuru
 * Modified by Daniella Niyonkuru (21/7/15 : Embedded CDBoost Version)
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

#ifndef BOOST_SIMULATION_PDEVS_ERUNNER_H
#define BOOST_SIMULATION_PDEVS_ERUNNER_H

#include <iostream>
#include <boost/simulation/pdevs/coordinator.hpp>
#include <boost/simulation/pdevs/driver.hpp>
#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>

#include "eTime.h"
#include "SWO.h"

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
class erunner
{
    TIME _next; //next scheduled event
    std::shared_ptr<coordinator<TIME, MSG, nullqueue>> _coordinator; //ecoordinator of the top level coupled model.
    std::shared_ptr<driver<TIME, MSG>> _driver; // global driver to manage top ports connected to hardware
    std::vector<std::pair<std::shared_ptr<port<TIME,MSG>>, std::shared_ptr<model<TIME>>>> _input_ports;
    std::vector<std::pair<std::shared_ptr<port<TIME,MSG>>, std::shared_ptr<model<TIME>>>> _output_ports;
    bool _silent;

    void process_output(TIME t, std::vector<MSG>& m) noexcept {
        for ( auto& msg : m){
        	MSG om = m.back(); // replace with msg
        	m.pop_back();
            _driver->send_hardware_command(om); // Send signal to hardware
        }
    }

    bool get_hardware_input(){
    	return false;
    }

    const TIME infinity;


public:
    //contructor

    /**
     * @brief eRunner constructing from a M model connected to an output.
     * @param cm is the coupled model to simulate.
     * @param ip is the set of top input ports.
     * @param op is the set of top output ports.
     */

	 explicit erunner(std::shared_ptr<coupled<TIME, MSG>> cm,
				std::vector<std::pair<std::shared_ptr<port<TIME,MSG>>, std::shared_ptr<model<TIME>>>> ip,
				std::vector<std::pair<std::shared_ptr<port<TIME,MSG>>, std::shared_ptr<model<TIME>>>> op) noexcept
	:    _input_ports(ip), _output_ports(op),infinity(cm->infinity)
    {
        for ( auto& inpport : ip){
            std::shared_ptr<port<TIME, MSG>> in_p = std::dynamic_pointer_cast<port<TIME, MSG>>(inpport.first);
            if ( in_p == nullptr){ //no input port provided
               printf("NULL PTR TO INPUT PORT \n");
             }
            else
            	in_p->print();
        }

        for ( auto& outpport : op){
            std::shared_ptr<port<TIME, MSG>> out_p = std::dynamic_pointer_cast<port<TIME, MSG>>(outpport.first);
            if ( out_p == nullptr){ //no output port provided
               printf("NULL PTR TO OUTPUT PORT \n");
             }
            else
            	out_p->print();
        }
        _coordinator.reset(new coordinator<TIME, MSG, nullqueue>{cm});
        _driver.reset(new driver<TIME,MSG>{ip,op});
        _next = _coordinator->init(TIME(00,00,00,010)); //TIME::currentTime()
        _silent = false;
    }


    /**
     * @brief runUntil starts the simulation and stops when the next event is scheduled after t.
     * @param t is the limit time for the simulation.
     * @return the TIME of the next event to happen when simulation stopped.
     */
    TIME runUntil(const TIME& t) noexcept
    {
    	Time::SetStartTime();
        if (_silent){
            while (_next < t)
            {
                _coordinator->advanceSimulation( _next);
                _next = _coordinator->next();
            }

        } else {
            while ((Time::currentTime() < t) || ( _next < t)) // should be while currentTime is less then t
            {
                while(Time::currentTime() < _next){

                	MSG input_message;
                	if((_driver->get_hardware_event(input_message))){ // Hardware Event Detected
                		_coordinator->postHardwareEvent(input_message);
                		_next = Time::currentTime();
                		break;
                	}
                }
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

               while(Time::currentTime() < _next){
                	MSG input_message;
                	if(_driver->get_hardware_event(input_message)){ // Hardware Event Detected
                		_coordinator->postHardwareEvent(input_message);
                		_next = Time::currentTime();
                		break;
                	}
                }
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


#endif // BOOST_SIMULATION_PDEVS_ERUNNER_H
