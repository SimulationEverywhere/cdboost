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


#ifndef BOOST_SIMULATION_PDEVS_COUPLED_H
#define BOOST_SIMULATION_PDEVS_COUPLED_H
#include <boost/simulation/pdevs/atomic.hpp>
#include <boost/simulation/model.hpp>

namespace boost {
namespace simulation {
namespace pdevs {

/**
 * @brief The coupled class represents PDEVS coupled models
 */
template<class TIME, class MSG>
class coupled : public model<TIME>
{
protected:
    /**
     * @brief The coupled_description struct provides the necesary data for Coordinators to construct the hierarchy of simulation
     */
    struct coupled_description{
        std::vector<std::shared_ptr<model<TIME>>> models;
        std::vector<std::shared_ptr<model<TIME>>> external_input_coupling;
        std::vector<std::pair<std::shared_ptr<model<TIME>>, std::shared_ptr<model<TIME>>>> internal_coupling; //first to second
        std::vector<std::shared_ptr<model<TIME>>> external_output_coupling;
    };

    coupled_description _desc;
public:
    using time_type=TIME;
    using message_type=MSG;
    using model_type=coupled<TIME, MSG>;
    using description_type=coupled_description;

    /**
     * @brief coupled receives the whole coupled model specs by pointers to models
     */
    coupled(std::initializer_list<std::shared_ptr<model<TIME>>> models,
            std::initializer_list<std::shared_ptr<model<TIME>>> eic,
            std::initializer_list<std::pair<std::shared_ptr<model<TIME>>, std::shared_ptr<model<TIME>>>> ic,
            std::initializer_list<std::shared_ptr<model<TIME>>> eoc
            ) noexcept
    {
          _desc.models = models;
          _desc.external_input_coupling = eic;
          _desc.internal_coupling = ic;
          _desc.external_output_coupling = eoc;

    }
    /**
     * @brief Coupled receives the whole coupled model spec by pointers to models
     * The difference with the other constructor is the use of vectors in place of initilizer_lists
     * for the case where the initializer_list can not be constructed (because using dynamic construction or MS compiler).
     */
    coupled(std::vector<std::shared_ptr<model<TIME>>> models,
            std::vector<std::shared_ptr<model<TIME>>> eic,
            std::vector<std::pair<std::shared_ptr<model<TIME>>, std::shared_ptr<model<TIME>>>> ic,
            std::vector<std::shared_ptr<model<TIME>>> eoc
             ) noexcept
    {
        _desc.models = models;
        _desc.external_input_coupling = eic;
        _desc.internal_coupling = ic;
        _desc.external_output_coupling = eoc;
    }
    /**
     * @brief get_description provides the model in a way a coordinator
     * can read to construct the simulation hierarchy.
     */
    coupled_description get_description() noexcept {
        //this is a good place for an assert that whole model is properly constructed
        return _desc;
    }

};



/**
 * @brief The flattened_coupled class represents a coupled model PDEVS that has a single level
 */
template<class TIME, class MSG>
class flattened_coupled : public coupled<TIME, MSG>
{
public:
    /**
     * @brief Coupled receives the whole coupled model spec
     * Each time a couple model is received it is exploted and put its components in current level.
     */
    flattened_coupled(std::initializer_list<std::shared_ptr<model<TIME>>> models,
            std::initializer_list<std::shared_ptr<model<TIME>>> eic,
            std::initializer_list<std::pair<std::shared_ptr<model<TIME>>, std::shared_ptr<model<TIME>>>> ic,
            std::initializer_list<std::shared_ptr<model<TIME>>> eoc
                          ) noexcept : coupled<TIME, MSG>({}, {}, {}, {})
    {
        for ( auto& model : models){
            std::shared_ptr<coupled<TIME, MSG>> m_coupled = std::dynamic_pointer_cast<coupled<TIME, MSG>>(model);
            if ( m_coupled == nullptr){ //an atomic
                coupled<TIME, MSG>::_desc.models.push_back(model);
            } else { //a coupled
                //inserting internal models
                for (auto& submodel : m_coupled->get_description().models){
                    coupled<TIME, MSG>::_desc.models.push_back(submodel);
                }
                //mantaining internal coups in submodels
                for (auto& subic : m_coupled->get_description().internal_coupling){
                    coupled<TIME, MSG>::_desc.internal_coupling.push_back(subic);
                }
            }
        }
        //connecting input links
        for (auto& in : eic){
            std::shared_ptr<coupled<TIME, MSG>> m_coupled = std::dynamic_pointer_cast<coupled<TIME, MSG>>(in);
            if ( m_coupled == nullptr){ //an atomic
                coupled<TIME, MSG>::_desc.external_input_coupling.push_back(in);
            } else {
                for (auto& subin : m_coupled->get_description().external_input_coupling){
                    coupled<TIME, MSG>::_desc.external_input_coupling.push_back(subin);
                }
            }
        }
        //connecting output links
        for (auto& out : eoc){
            std::shared_ptr<coupled<TIME, MSG>> m_coupled = std::dynamic_pointer_cast<coupled<TIME, MSG>>(out);
            if ( m_coupled == nullptr){ //an atomic
                coupled<TIME, MSG>::_desc.external_output_coupling.push_back(out);
            } else {
                for (auto& subout : m_coupled->get_description().external_output_coupling){
                    coupled<TIME, MSG>::_desc.external_output_coupling.push_back(subout);
                }
            }
        }
        //connecting internal links
        for (auto& coupling : ic){
            std::shared_ptr<coupled<TIME, MSG>> m_left = std::dynamic_pointer_cast<coupled<TIME, MSG>>(coupling.first);
            std::shared_ptr<coupled<TIME, MSG>> m_right = std::dynamic_pointer_cast<coupled<TIME, MSG>>(coupling.second);
            if ( m_left == nullptr){ //left is an atomic
                if (m_right == nullptr) { //and right is an atomic
                    coupled<TIME, MSG>::_desc.internal_coupling.push_back(coupling);
                } else { //and right is a coupled
                    for (auto& right_in : m_right->get_description().external_input_coupling){
                        coupled<TIME, MSG>::_desc.internal_coupling.push_back({coupling.first, right_in});
                    }
                }
            } else { // left is a coupled
                if (m_right == nullptr){ //and right is atomic
                    for (auto& subout : m_left->get_description().external_output_coupling){
                        coupled<TIME, MSG>::_desc.internal_coupling.push_back({subout, m_right});
                    }
                } else { // and right is coupled
                    for (auto& left_out : m_left->get_description().external_output_coupling){
                        for (auto& right_in : m_right->get_description().external_input_coupling){
                            coupled<TIME, MSG>::_desc.internal_coupling.push_back({left_out, right_in});
                        }
                    }
                }
            }

        }
    }
    /**
     * @brief Coupled receives the whole coupled model spec
     * Each time a couple model is received it is exploted and put its components in current level.
     * The difference with the other constructor is the use of vectors in place of initilizer_lists
     * for the case where the initializer_list can not be constructed (because using dynamic construction or MS compiler).
     */
    flattened_coupled(std::vector<std::shared_ptr<model<TIME>>> models,
            std::vector<std::shared_ptr<model<TIME>>> eic,
            std::vector<std::pair<std::shared_ptr<model<TIME>>, std::shared_ptr<model<TIME>>>> ic,
            std::vector<std::shared_ptr<model<TIME>>> eoc
             ) noexcept : coupled<TIME, MSG>({}, {}, {}, {})
    {
        for ( auto& model : models){
            std::shared_ptr<coupled<TIME, MSG>> m_coupled = std::dynamic_pointer_cast<coupled<TIME, MSG>>(model);
            if ( m_coupled == nullptr){ //an atomic
                coupled<TIME, MSG>::_desc.models.push_back(model);
            } else { //a coupled
                //inserting internal models
                for (auto& submodel : m_coupled->get_description().models){
                    coupled<TIME, MSG>::_desc.models.push_back(submodel);
                }
                //mantaining internal coups in submodels
                for (auto& subic : m_coupled->get_description().internal_coupling){
                    coupled<TIME, MSG>::_desc.internal_coupling.push_back(subic);
                }
            }
        }
        //connecting input links
        for (auto& in : eic){
            std::shared_ptr<coupled<TIME, MSG>> m_coupled = std::dynamic_pointer_cast<coupled<TIME, MSG>>(in);
            if ( m_coupled == nullptr){ //an atomic
                coupled<TIME, MSG>::_desc.external_input_coupling.push_back(in);
            } else {
                for (auto& subin : m_coupled->get_description().external_input_coupling){
                    coupled<TIME, MSG>::_desc.external_input_coupling.push_back(subin);
                }
            }
        }
        //connecting output links
        for (auto& out : eoc){
            std::shared_ptr<coupled<TIME, MSG>> m_coupled = std::dynamic_pointer_cast<coupled<TIME, MSG>>(out);
            if ( m_coupled == nullptr){ //an atomic
                coupled<TIME, MSG>::_desc.external_output_coupling.push_back(out);
            } else {
                for (auto& subout : m_coupled->get_description().external_output_coupling){
                    coupled<TIME, MSG>::_desc.external_output_coupling.push_back(subout);
                }
            }
        }
        //connecting internal links
        for (auto& coupling : ic){
            std::shared_ptr<coupled<TIME, MSG>> m_left = std::dynamic_pointer_cast<coupled<TIME, MSG>>(coupling.first);
            std::shared_ptr<coupled<TIME, MSG>> m_right = std::dynamic_pointer_cast<coupled<TIME, MSG>>(coupling.second);
            if ( m_left == nullptr){ //left is an atomic
                if (m_right == nullptr) { //and right is an atomic
                    coupled<TIME, MSG>::_desc.internal_coupling.push_back(coupling);
                } else { //and right is a coupled
                    for (auto& right_in : m_right->get_description().external_input_coupling){
                        coupled<TIME, MSG>::_desc.internal_coupling.push_back({coupling.first, right_in});
                    }
                }
            } else { // left is a coupled
                if (m_right == nullptr){ //and right is atomic
                    for (auto& subout : m_left->get_description().external_output_coupling){
                        coupled<TIME, MSG>::_desc.internal_coupling.push_back({subout, m_right});
                    }
                } else { // and right is coupled
                    for (auto& left_out : m_left->get_description().external_output_coupling){
                        for (auto& right_in : m_right->get_description().external_input_coupling){
                            coupled<TIME, MSG>::_desc.internal_coupling.push_back({left_out, right_in});
                        }
                    }
                }
            }

        }
    }
    /**
     * @brief get_description provides the model in a way a coordinator
     * can read to construct the simulation hierarchy.
     */
    typename coupled<TIME, MSG>::coupled_description get_description() noexcept {
        return coupled<TIME, MSG>::_desc;
    }

};

}
}
}

#endif // BOOST_SIMULATION_PDEVS_COUPLED_H
