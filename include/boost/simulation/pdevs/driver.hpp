/**
 * Copyright (c) 2015, Daniella Niyonkuru
 * Carleton University
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


#ifndef BOOST_SIMULATION_PDEVS_DRIVER_H
#define BOOST_SIMULATION_PDEVS_DRIVER_H
#include <boost/simulation/pdevs/port.hpp>
#include <boost/simulation/model.hpp>
#include "mbed.h"
#include "SWO.h"

namespace boost {
namespace simulation {
namespace pdevs {

/**
 * @brief The coupled class represents PDEVS coupled models
 */
template<class TIME, class MSG>
class driver
{
protected:
    /**
     * @brief The topports_description struct provides the necessary data to retrieve the top ports
     */
	int numberInputPorts;
	std::vector<TIME> previousEventTime;
    struct topports_description{
        std::vector<std::pair<std::shared_ptr<port<TIME,MSG>>, std::shared_ptr<model<TIME>>>> input_ports; //port to model
        std::vector<std::pair<std::shared_ptr<port<TIME,MSG>>, std::shared_ptr<model<TIME>>>> output_ports; // model to port
    };

    topports_description _ports_desc;
public:
    using time_type=TIME;
    using message_type=MSG;
    using description_type=topports_description;
    using Value=int;


    /**
     * @brief driver receives the whole port structure by pointers to ports
     */
    driver( std::vector<std::pair<std::shared_ptr<port<TIME,MSG>>, std::shared_ptr<model<TIME>>>> ip,
		     std::vector<std::pair<std::shared_ptr<port<TIME,MSG>>, std::shared_ptr<model<TIME>>>> op
          ) noexcept
    {
        _ports_desc.input_ports = ip;
        _ports_desc.output_ports = op;
        numberInputPorts = _ports_desc.input_ports.size();
        previousEventTime.assign(numberInputPorts,TIME::currentTime());
        //SWO_PrintString("GLOBAL DRIVER CREATED \n");
    }
    /**
     * @brief get_description provides the ports in a way a coordinator
     * can read to construct the port connection.
     */
    topports_description get_description() noexcept {
        //this is a good place for an assert that port structure is well constructed
        return _ports_desc;
    }

    bool get_hardware_event(MSG& input_message){
        Value portValue;
        std::shared_ptr<model<TIME>> to_model;
        std::shared_ptr<port<TIME,MSG>> current_port;


        bool foundEvent = false;
    	for (auto& ip : _ports_desc.input_ports){
    	    current_port = std::dynamic_pointer_cast<port<TIME, MSG>>(ip.first);

    	    int portIndex = &ip - &_ports_desc.input_ports[0]; // pos contains the position in the vector

    	    if(Time::currentTime() < previousEventTime[portIndex] + current_port->getPollingPeriod())
    	    	continue; // Go to the next port - it's not time to poll this port yet ;)

    		if(current_port->pDriver(portValue)){
    			to_model = std::dynamic_pointer_cast<model<TIME>>(ip.second);
    			foundEvent = true;
    			previousEventTime[portIndex] = TIME::currentTime();
    			break;
    		}
    	}
    	if(foundEvent){
    		input_message.tm = TIME::currentTime();
    		input_message.to = to_model;
    		input_message.port = current_port->asString();
    		input_message.val = portValue;
    		// Debugging purposes
    		// SWO_PrintString("DRIVER: INPUT MESSAGE \n");
    		input_message.print();

    		return true;
    	}
    	else {
    		return false;
    	}
    }

    bool send_hardware_command(MSG& output_message){
    	std::string port_name = output_message.port;
    	// For debug purposes only
    	// SWO_PrintString("DRIVER: OUTPUT MESSAGE \n");
		output_message.print();

    	Value portValue = output_message.val;
    	for(auto& op : _ports_desc.output_ports){
    		if(op.first->asString() == port_name){
    			op.first->pDriver(portValue);
    		}
    	}
    	return true;
    }

    void initHardware();
    void closeHardware();

};


}
}
}

#endif // BOOST_SIMULATION_PDEVS_COUPLED_H
