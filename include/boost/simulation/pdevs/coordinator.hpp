/**
 * Copyright (c) 2013-2015, Damian Vicino, Daniella Niyonkuru
 * Modified by Daniella Niyonkuru for the Embedded CDBoost version
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


#ifndef BOOST_SIMULATION_PDEVS_COORDINATOR_H
#define BOOST_SIMULATION_PDEVS_COORDINATOR_H
#include <map>
#include <queue>
#include <cassert>

#include <boost/simulation/pdevs/coupled.hpp>
#include <boost/any.hpp>
#include "SWO.h"
#include "mbed.h"

namespace boost {
namespace simulation {
namespace pdevs {


template<class, class>
class nullqueue{
    //this queue is passed as template parameter to the coordinator to tell it to
    //avoid using queues and run pooling time advance from models.
};

/**
 * @brief The Coordinator class runs a PDEVS coupled model
 * The Coordinators are used to run the coupled models.
 * At the time the coupled model is assigned to the Coordinator it creates
 * other coordinators and simulators to handle the submodels of the coupled
 * model and then it coordinates the advance of all these coordinators and
 * simulators to provide its own outputs.
 * This kind of coordinator advances time by small certain steps.
 * There is never a rollback.
 * Each call to advanceSimulation advances internally a step and outputs are collected in separate method.
 * Time and Message are the representations for time and message, FEL is the structure to represent Future Event List
 */

//FEL needs a anything with the same operations as std::priority_queue<std::pair<TIME, std::shared_ptr<PCoordinator<TIME, MSG, FEL>>>
template <class VALUE_TYPE, class COMPARE_TYPE>
using priority_queue_vector = std::priority_queue<VALUE_TYPE, std::vector<VALUE_TYPE>, COMPARE_TYPE>;

template<class TIME, class MSG, template<class, class> class FEL=nullqueue>//nullqueue FEL means no structure and pool from models.
class coordinator
{
   //We assume that FEL interface is compatible to queue.h interface for now.
   //also we assume there is not cheap way to do the removal of elements when changing schedule in a model.

    TIME _last; //last transition time
    TIME _next; // next transition scheduled
    //used when coordinating
    std::vector<std::shared_ptr<coordinator<TIME, MSG, FEL>>> _subcoordinators;
    //used when simulating
    std::shared_ptr<atomic<TIME, MSG>> _model; // atomic model simulated
    //coupling in this coordinator
    std::vector<std::shared_ptr<coordinator<TIME, MSG, FEL>>> _external_input_coupling;
    //couplings from uplevel coordinator
    bool _is_connected_to_out; //tell if connected to output of the next level coupled model.
    std::vector<std::shared_ptr<coordinator<TIME, MSG, FEL>>> _internal_connections; // tell what models are the message going in the next level coupled.
    //infinity of current time representation
    TIME infinity;
    //_inbox next level model puts here what will be consumed in next advanceSimulation call
    std::vector<MSG> _inbox;
    //Future Event List
    using FEL_ITEM_TYPE = std::pair<TIME, std::shared_ptr<coordinator<TIME, MSG, FEL>>>;
    using FEL_COMP_TYPE = bool(*)( const FEL_ITEM_TYPE &lhs, const FEL_ITEM_TYPE &rhs);
    FEL<FEL_ITEM_TYPE, FEL_COMP_TYPE> _fel;

    std::vector<std::shared_ptr<coordinator<TIME, MSG, FEL>>> _inminents;
public:
    coordinator() = delete;
    /**
     * @brief Coordinator constructs from an PCoupled model.
     * @param a pointer to the Coupled model simulated.
     */
    explicit coordinator(std::shared_ptr<coupled<TIME, MSG>> c) noexcept
        : _model(nullptr), _is_connected_to_out(false), _internal_connections(), infinity(c->infinity)
    {
       //initialize FEL
       auto FELcomp = [](const FEL_ITEM_TYPE &lhs, const FEL_ITEM_TYPE &rhs){ return lhs.first > rhs.first ; };
       _fel = FEL<FEL_ITEM_TYPE, FEL_COMP_TYPE>(FELcomp);


       // from here on similar in all specializations
       auto desc = c->get_description();
       std::map<void*, std::shared_ptr<coordinator<TIME, MSG, FEL>>> model_to_container; //using void* to only check address match
       for (auto& m : desc.models){
           bool to_external_out;
           //is current model connected to output?
           to_external_out = false;
           if(std::any_of(desc.external_output_coupling.begin(),
                          desc.external_output_coupling.end(),
                          [&m](std::shared_ptr<model<TIME>>& eoc){
                                return eoc ==  m; }
                          )){
                                to_external_out = true;
                            }
           //create coordinators and simulators
           std::shared_ptr<atomic<TIME, MSG>> m_atomic = std::dynamic_pointer_cast<atomic<TIME, MSG>>(m);
           std::shared_ptr<coupled<TIME, MSG>> m_coupled = std::dynamic_pointer_cast<coupled<TIME, MSG>>(m);

           if (m_atomic == nullptr){
               assert(m_coupled != nullptr);
               auto coord = std::make_shared<coordinator<TIME, MSG, FEL>>(m_coupled);
               coord->_is_connected_to_out=to_external_out;
               _subcoordinators.push_back(coord);
               model_to_container.emplace((void*) m.get(), coord );
           } else {
               auto sim = std::make_shared<coordinator<TIME, MSG, FEL>>(m_atomic);
               sim->_is_connected_to_out=to_external_out;
               _subcoordinators.push_back(sim);
               model_to_container.emplace((void*) m.get(), sim );
           }
       }
       //remap interconnections
       //internal connections
       for (auto& m : desc.models){
           std::vector<std::shared_ptr<coordinator<TIME, MSG, FEL>>> to_internals;
           to_internals.clear();
           for (auto& ints : desc.internal_coupling){

               if (ints.first.get() ==  m.get()){
                   //inserting destination
                   std::shared_ptr<atomic<TIME, MSG>> int_sec_atomic = std::dynamic_pointer_cast<atomic<TIME, MSG>>(ints.second);
                   std::shared_ptr<coupled<TIME, MSG>> int_sec_coupled = std::dynamic_pointer_cast<coupled<TIME, MSG>>(ints.second);

                   if(int_sec_atomic == nullptr){ //coupled model destination
                       assert(int_sec_coupled != nullptr);
                       to_internals.push_back(model_to_container[int_sec_coupled.get()]);
                   } else { //destination atomic
                       to_internals.push_back(model_to_container[int_sec_atomic.get()]);
                   }
               }
           }
           //assigning
            model_to_container[m.get()]->_internal_connections = to_internals;
       }
       //external_input_coupling
       for (auto& a : desc.external_input_coupling){
            _external_input_coupling.push_back( model_to_container[a.get()] );
       }
    }

    /**
     * @brief Coordinator for simulation constructs from an PAtomic model.
     * @param a pointer to the Atomic model simulated.
     */
    explicit coordinator(std::shared_ptr<atomic<TIME, MSG>> a) noexcept : _model(a), infinity(a->infinity) {}

    /**
     * @brief Coordinator expected next internal transition time
     */
    TIME next() const noexcept {
        return _next;
    }

    /**
     * @brief init function sets the start time
     * @param t is the start time
     * @return the time until first time advance result
     */
    TIME init(TIME t) noexcept {
        _last = t;
        //init all submodels and find next transition time
        _next = infinity;
        if (_model != nullptr){ //if need to simulate a model
            _next = _last + _model->advance();
        } else { //if need to run a pure coordinator
            //init submodles and queue them if next internal event is not infinity
            for ( auto& c: _subcoordinators){
                TIME next = c->init(t);
                if (next < _next) _next = next;
                if (next != infinity) _fel.emplace(next, c);
            }
        }
        //setup inminents
        while(!_fel.empty() && _fel.top().first == _next){
            _inminents.push_back(_fel.top().second);
            _fel.pop();
        }

        return _next;
    }
    /**
     * @brief advanceSimulation advances the execution to t, at t introduces the messages into the system (if any).
     * @param t is the time the transition is expected to be run.
     * @return the time until next internal event.
     */
    void advanceSimulation(const TIME& t) noexcept { //bag of input was collected in _inbox internal var.
        //if model is simulated in this coordinator
    	// For debug purposes only
    	/*
    	SWO_PrintString((_model->asString()).c_str());
    	SWO_PrintString(" - Advance Execution Call \n");
    	*/
        if (_model != nullptr){
            assert(t >= _last);
            assert(t <= _next );
            if (_inbox.empty()){
                if (t == _next){
                    _model->internal();
                    _last = t;
                    _next = _last + _model->advance();
                } else {
                    _last = t;
                }
            } else {
                if ( t == _next){ //confluence
                    _model->confluence(_inbox, t-_last);
                    _last = t;
                    _next = _last + _model->advance();
                } else { //external
                    _model->external(_inbox, t-_last);
                    _last = t;
                    _next = _last + _model->advance();
                }
            }
        } else {  //if coordinator is pure and no model is simulated
            assert(t <= _next);
            assert(t >= _last);
            _last = t;
            //std::vector<std::shared_ptr<PCoordinator<TIME, MSG, FEL>>> inminents_internal;
            std::vector<std::shared_ptr<coordinator<TIME, MSG, FEL>>> inminents_external;
            //processing external input into _inboxes
            for (auto& receiver : _external_input_coupling){
                    if (receiver->next() != _last && receiver->_inbox.size() == 0){
                        inminents_external.push_back(receiver);
                    }
                    receiver->_inbox.insert(receiver->_inbox.end(), _inbox.begin(), _inbox.end());
            }
            //collecting inputs and adding inminents models for internal transitions
            if (_last == _next) {
                for (auto& co : _inminents){
                    if (co->_internal_connections.size()){
                        std::vector<MSG> out = co->collectOutputs(_last);
                        for (auto& receiver : co->_internal_connections){
                            if (receiver->next() != _last && receiver->_inbox.size() == 0){
                                inminents_external.push_back(receiver);
                            }
                            receiver->_inbox.insert(receiver->_inbox.end(), out.begin(), out.end());
                        }
                    }
                }
            }
            //processing inminents
            for (auto& co : _inminents){
                co->advanceSimulation(t);
                if (co->next() != infinity) _fel.emplace(co->next(), co);
            }
            for (auto& co : inminents_external){
                co->advanceSimulation(t);
                if (co->next() != infinity) _fel.emplace(co->next(), co);
            }
            //setting up next variable
            _next = infinity;
            //consuming the queue, skip the replaced events
            while (_next == infinity && !_fel.empty()){
                if (_fel.top().first == _fel.top().second->next()){
                    _next = _fel.top().first;
                } else {
                    _fel.pop();
                }
            }
        }
        _inbox.clear();
        //setup next inminents
        _inminents.clear();
        while(!_fel.empty() && _fel.top().first == _next){
            if (_fel.top().second->next() == _next){
                _inminents.push_back(_fel.top().second);
            }
            _fel.pop();
        }

    }

    std::vector<MSG> collectOutputs(const TIME& t) noexcept {
    	// For debug purposes only
    	/*
    	SWO_PrintString((_model->asString()).c_str());
    	SWO_PrintString(" - Collect Outputs Call \n");
        */
        if (_next != t) return {}; //not my turn

        if (_model != nullptr){ return _model->out(); } //atomic model

        //if coordinator of coupled model
        std::vector<MSG> vm;
        for (auto& co : _inminents){
            if (co->next() == t && co->_is_connected_to_out){
                std::vector<MSG> tmp = co->collectOutputs(t);
                vm.insert(vm.end(), tmp.begin(), tmp.end());
            }
        }
        return vm;
    }

};


//specialiazation for pooling to models in place of using a FEL.
template<class TIME, class MSG>
class coordinator<TIME, MSG, nullqueue>
{
    TIME _last; //last transition time
    TIME _next; // next transition scheduled
    //used when coordinating
    std::vector<std::shared_ptr<coordinator<TIME, MSG, nullqueue>>> _subcoordinators;
    //used when simulating
    std::shared_ptr<atomic<TIME, MSG>> _model; // atomic model simulated
    //coupling in this coordinator
    std::vector<std::shared_ptr<coordinator<TIME, MSG, nullqueue>>> _external_input_coupling;
    //couplings from uplevel coordinator
    bool _is_connected_to_out; //tell if connected to output of the next level coupled model.
    std::vector<std::shared_ptr<coordinator<TIME, MSG, nullqueue>>> _internal_connections; // tell what models are the message going in the next level coupled.
    //infinity of current time representation
    TIME infinity;
    //_inbox next level model puts here what will be consumed in next advanceSimulation call
    std::vector<MSG> _inbox;
    //caching output
    int _processed_output = 0;
    int _processed_advances = 0;
    std::vector<MSG> _cached_out;
public:
    coordinator() = delete;
    /**
     * @brief Coordinator constructs from an PCoupled model.
     * @param a pointer to the Coupled model simulated.
     */
    explicit coordinator(std::shared_ptr<coupled<TIME, MSG>> c) noexcept
        : _model(nullptr), _is_connected_to_out(false), _internal_connections(), infinity(c->infinity)
    {
       auto desc = c->get_description();
       std::map<void*, std::shared_ptr<coordinator<TIME, MSG, nullqueue>>> model_to_container; //using void* to only check address match
       for (auto& m : desc.models){
           bool to_external_out;
           //is current model connected to output?
           to_external_out = false;
           if(std::any_of(desc.external_output_coupling.begin(),
                          desc.external_output_coupling.end(),
                          [&m](std::shared_ptr<model<TIME>>& eoc){
                                return eoc ==  m; }
                          )){
                                to_external_out = true;
                            }
           //create coordinators and simulators
           std::shared_ptr<atomic<TIME, MSG>> m_atomic = std::dynamic_pointer_cast<atomic<TIME, MSG>>(m);
           std::shared_ptr<coupled<TIME, MSG>> m_coupled = std::dynamic_pointer_cast<coupled<TIME, MSG>>(m);
           if ( m_atomic == nullptr){
               assert(m_coupled != nullptr);
               auto coord = std::make_shared<coordinator<TIME, MSG, nullqueue>>(m_coupled);
               coord->_is_connected_to_out=to_external_out;
               _subcoordinators.push_back(coord);
               model_to_container.emplace((void*) m_coupled.get(), coord );
           } else {
               auto sim = std::make_shared<coordinator<TIME, MSG, nullqueue>>(m_atomic);
               sim->_is_connected_to_out=to_external_out;
               _subcoordinators.push_back(sim);
               model_to_container.emplace((void*) m_atomic.get(), sim );
           }
       }
       //remap interconnections
       //internal connections
       for (auto& m : desc.models){
           std::vector<std::shared_ptr<coordinator<TIME, MSG, nullqueue>>> to_internals;
           to_internals.clear();
           for (auto& ints : desc.internal_coupling){
               if (ints.first.get() ==  m.get() ){
                   //inserting destination
                   std::shared_ptr<atomic<TIME, MSG>> int_sec_atomic = std::dynamic_pointer_cast<atomic<TIME, MSG>>(ints.second);
                   std::shared_ptr<coupled<TIME, MSG>> int_sec_coupled = std::dynamic_pointer_cast<coupled<TIME, MSG>>(ints.second);
                   if(int_sec_atomic == nullptr){ //coupled model destination
                       assert(int_sec_coupled != nullptr);
                       to_internals.push_back(model_to_container[int_sec_coupled.get()]);
                   } else { //destination atomic
                       to_internals.push_back(model_to_container[int_sec_atomic.get()]);
                   }
               }
           }
           //assigning
           model_to_container[m.get()]->_internal_connections = to_internals;
       }
       //external_input_coupling
       for (auto& a : desc.external_input_coupling){
           _external_input_coupling.push_back( model_to_container[a.get()] );
       }
    }

    /**
     * @brief Coordinator for simulation constructs from an PAtomic model.
     * @param a pointer to the Atomic model simulated.
     */
    explicit coordinator(std::shared_ptr<atomic<TIME, MSG>> a) noexcept : _model(a), infinity(a->infinity) {}
    /**
     * @brief Coordinator expected next internal transition time
     */
    TIME next() const noexcept {
        return _next;
    }

    /**
     * @brief init function sets the start time
     * @param t is the start time
     * @return the time until first time advance result
     */
    TIME init(TIME t) noexcept {
        _last = t;
        //init all submodels and find next transition time.
        _next = infinity;
        if (_model != nullptr){ //if need to simulate a model
            _next = _last + _model->advance();
        } else { //if need to run a pure coordinator
            for ( auto& c: _subcoordinators){
                TIME next = c->init(t);
                if (next < _next) _next = next;
            }
        }
        return _next;
    }
    /**
     * @brief postHardwareEvent adds a message to the inbox of a coordinator. This action is too be triggered by the runner.
     * @param m is the message to be added to the inbox.
     * @return void.
     */
    void postHardwareEvent(MSG m)noexcept{
    	_inbox.push_back(m); // considering we are pushing one event now (Embedded CD-Boost)
    }
    /**
     * @brief advanceSimulation advances the execution to t, at t introduces the messages into the system (if any).
     * @param t is the time the transition is expected to be run.
     * @return the time until next internal event.
     */
    void advanceSimulation(const TIME& t) noexcept { //bag of input was collected in _inbox internal var.
    	// For debug purposes only
    	//SWO_PrintString(" - advance_execution()::");

        _processed_advances++; //invalidate cached output
        //if model is atomic - this is a simulator -> Execute Simulator algos
        if (_model != nullptr){
        	_model->print();
            assert(t >= _last);
            assert(t <= _next );
            if (_inbox.empty()){
                if (t == _next){
                    _model->internal();
                    _last = t;
                    _next = _last + _model->advance();
                    //SWO_PrintString(("\t model->internal() model->advance(): " + _model->advance().asString() + " \n").c_str());
                } else {
//                    throw std::exception();
                    _last = t;
                }
            } else {
                if ( t == _next){ //confluence
                    _model->confluence(_inbox, t-_last);
                    _last = t;
                    _next = _last + _model->advance();
                    //SWO_PrintString("\t model->confluent() model->advance() \n");
                } else { //external
                    _model->external(_inbox, t-_last);
                    _last = t;
                    _next = _last + _model->advance();
                    //SWO_PrintString(("\t model->external() model->advance(): " + _model->advance().asString() + " \n").c_str());
                }
            }
        } else {  ////if model is coupled- this is a pure coordinator -> Execute Coordinator algos
        	SWO_PrintString("flattop \n");
            assert(t <= _next);
            assert(t >= _last);
            _last = t;
            std::vector<std::shared_ptr<coordinator<TIME, MSG, nullqueue>>> inminents_internal;
            std::vector<std::shared_ptr<coordinator<TIME, MSG, nullqueue>>> inminents_external;
            //processing external input into _inboxes
            for (auto& receiver : _external_input_coupling){
                    if (receiver->next() != _last && receiver->_inbox.size() == 0){ // was == 0
                        inminents_external.push_back(receiver);
                    }
                    receiver->_inbox.insert(receiver->_inbox.end(), _inbox.begin(), _inbox.end());
            }
            //collecting inputs and adding inminents models for internal transitions
            if (_last == _next) {
                for (auto& co : _subcoordinators){
                    if (co->next() == _next){//has internal waiting
                        inminents_internal.push_back(co);
                        if (co->_internal_connections.size()){
                            std::vector<MSG> out = co->collectOutputs(_last);
                            for (auto& receiver : co->_internal_connections){
                                    if (receiver->next() != _last && receiver->_inbox.size() == 0){
                                        inminents_external.push_back(receiver);
                                    }
                                    receiver->_inbox.insert(receiver->_inbox.end(), out.begin(), out.end());
                            }
                        }
                    }
                }
            }
            //processing inminents
            for (auto& co : inminents_internal){
                co->advanceSimulation(t);
            }
            for (auto& co : inminents_external){
                co->advanceSimulation(t);

            }
            //setting up next variable
            auto next_coord = std::min_element(_subcoordinators.begin(), _subcoordinators.end(),
                                     [](std::shared_ptr<coordinator>& pc1, std::shared_ptr<coordinator>& pc2){ return pc1->next() < pc2->next();});
            _next = (*next_coord)->next();
        }
        _inbox.clear();
    }

    std::vector<MSG> collectOutputs(const TIME& t) noexcept {
    	// For debug purposes only
    	// SWO_PrintString(" - collect_outputs()::");

        if (_next != t) return {}; //not my turn

        //SWO_PrintString(" - collect_outputs()::");
        if (_model != nullptr){
        	/*_model->print(); SWO_PrintString("\t model->out() \n");*/
        	return _model->out();
        } //atomic model
        //else { SWO_PrintString("flattop \n");}

        //if coordinator of coupled model
        std::vector<MSG> vm;
        for (auto& co : _subcoordinators){
            if (co->next() == t && co->_is_connected_to_out){
                std::vector<MSG> tmp = co->collectOutputs(t);
                vm.insert(vm.end(), tmp.begin(), tmp.end());
            }
        }
        return vm;
    }

};


}
}
}

#endif // BOOST_SIMULATION_PDEVS_COORDINATOR_H
