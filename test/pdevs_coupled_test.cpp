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


#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/simulation/pdevs/coupled.hpp>
#include <boost/simulation/pdevs/basic_models/generator.hpp>
#include <boost/simulation/pdevs/basic_models/infinite_counter.hpp>
#include <boost/rational.hpp>
#include <boost/simulation/convenience.hpp>

using namespace boost::simulation;
using namespace boost::simulation::pdevs;
using namespace boost::simulation::pdevs::basic_models;

using Time=boost::rational<int>;
using Message=boost::any;
BOOST_AUTO_TEST_SUITE( p_coupled_test_suite )

BOOST_AUTO_TEST_SUITE( p_coupled_using_initializer_list_test_suite )
BOOST_AUTO_TEST_CASE( p_generator_into_coupled_initializer_list_coupling_test )
{
    auto pg = make_atomic_ptr<generator<Time, Message>, Time>(Time{1});
    coupled<Time, Message> pc{
        {pg}, {}, {}, {pg}
    };
    coupled<Time, Message>::description_type desc =  pc.get_description();
    BOOST_CHECK_EQUAL(desc.models.size(), 1);

    std::shared_ptr<atomic<Time, Message>> m_atomic = std::dynamic_pointer_cast<atomic<Time, Message>>(desc.models[0]);
    BOOST_CHECK_EQUAL(m_atomic, pg);
    BOOST_CHECK_EQUAL(desc.external_input_coupling.size(), 0);
    BOOST_CHECK_EQUAL(desc.internal_coupling.size(), 0);
    BOOST_CHECK_EQUAL(desc.external_output_coupling.size(), 1);

    m_atomic = std::dynamic_pointer_cast<atomic<Time, Message>>(desc.external_output_coupling[0]);
    BOOST_CHECK_EQUAL(m_atomic, pg);
}
BOOST_AUTO_TEST_CASE( p_generator_to_p_infinite_counter_into_coupled_initializer_list_coupling_test )
{
    auto pg = make_atomic_ptr<generator<Time, Message>, Time>(Time{1});
    auto pic = make_atomic_ptr<infinite_counter<Time, Message>>();
    coupled<Time, Message> pc{
        {pg, pic}, {pic}, {{pg, pic}}, {pic}
    };

    coupled<Time, Message>::description_type desc =  pc.get_description();
    BOOST_CHECK_EQUAL(desc.models.size(), 2);
    BOOST_CHECK_EQUAL(std::count_if(desc.models.begin(), desc.models.end(),
                                    [&pg](std::shared_ptr<model<Time>>& modelptr){ return modelptr == pg; }), 1);
    BOOST_CHECK_EQUAL(std::count_if(desc.models.begin(), desc.models.end(),
                                    [&pic](std::shared_ptr<model<Time>>& modelptr){ return modelptr == pic; }), 1);
    BOOST_CHECK_EQUAL(desc.external_input_coupling.size(), 1);
    BOOST_CHECK_EQUAL(std::count_if(desc.external_input_coupling.begin(), desc.external_input_coupling.end(),
                                    [&pic](std::shared_ptr<model<Time>> & modelptr){ return modelptr == pic; }), 1);
    BOOST_CHECK_EQUAL(desc.internal_coupling.size(), 1);
    BOOST_CHECK_EQUAL(std::count_if(desc.internal_coupling.begin(), desc.internal_coupling.end(),
                                    [&pg, &pic](std::pair<std::shared_ptr<model<Time>>, std::shared_ptr<model<Time>>> & coupling){ return coupling.first == pg &&
                                           coupling.second == pic;})
                      , 1);
    BOOST_CHECK_EQUAL(desc.external_output_coupling.size(), 1);
    BOOST_CHECK_EQUAL(std::count_if(desc.external_output_coupling.begin(), desc.external_output_coupling.end(),
                                    [&pic](std::shared_ptr<model<Time>>& modelptr){ return modelptr == pic; }), 1);
}
BOOST_AUTO_TEST_CASE( p_generator_to_p_infinite_counter_into_coupleds_and_coupled_initializer_list_coupling_test )
{
    auto pg1 = make_atomic_ptr<generator<Time, Message>, Time>(Time{1});
    auto pg2 = make_atomic_ptr<generator<Time, Message>, Time>(Time{1});
    auto pic = make_atomic_ptr<infinite_counter<Time, Message>>();
    auto pc1 = std::shared_ptr<coupled<Time, Message>>( new coupled<Time, Message>{
        {pg1, pic}, {pic}, {{pg1, pic}}, {pic}
    });
    coupled<Time, Message> pc2{
        {pg2, pc1}, {pc1}, {{pg2, pc1}}, {pc1}
    };

    //check pc2 model description
    coupled<Time, Message>::description_type desc =  pc2.get_description();
    BOOST_CHECK_EQUAL(desc.models.size(), 2);
    BOOST_CHECK_EQUAL(std::count_if(desc.models.begin(), desc.models.end(),
                                    [&pg2](std::shared_ptr<model<Time>>& modelptr){ return modelptr == pg2; }), 1);
    BOOST_CHECK_EQUAL(std::count_if(desc.models.begin(), desc.models.end(),
                                    [&pc1](std::shared_ptr<model<Time>>& modelptr){ return modelptr == pc1; }), 1);
    BOOST_CHECK_EQUAL(desc.external_input_coupling.size(), 1);
    BOOST_CHECK_EQUAL(std::count_if(desc.external_input_coupling.begin(), desc.external_input_coupling.end(),
                                    [&pc1](std::shared_ptr<model<Time>>& modelptr){ return modelptr == pc1; }), 1);
    BOOST_CHECK_EQUAL(desc.internal_coupling.size(), 1);
    BOOST_CHECK_EQUAL(std::count_if(desc.internal_coupling.begin(), desc.internal_coupling.end(),
                                    [&pg2, &pc1](std::pair<std::shared_ptr<model<Time>>, std::shared_ptr<model<Time>>> & coupling){
                                           return coupling.first == pg2 && coupling.second == pc1 ;})
                      , 1);
    BOOST_CHECK_EQUAL(desc.external_output_coupling.size(), 1);
    BOOST_CHECK_EQUAL(std::count_if(desc.external_output_coupling.begin(), desc.external_output_coupling.end(),
                                    [&pc1](std::shared_ptr<model<Time>>& modelptr){ return  modelptr == pc1; }), 1);
    //check pc1 model description
    coupled<Time, Message>::description_type desc_int =  std::dynamic_pointer_cast<coupled<Time, Message>>( desc.models[1] )->get_description();
    BOOST_CHECK_EQUAL(desc_int.models.size(), 2);
    BOOST_CHECK_EQUAL(std::count_if(desc_int.models.begin(), desc_int.models.end(),
                                    [&pg1](std::shared_ptr<model<Time>>& modelptr){ return modelptr == pg1 ; }), 1);
    BOOST_CHECK_EQUAL(std::count_if(desc_int.models.begin(), desc_int.models.end(),
                                    [&pic](std::shared_ptr<model<Time>>& modelptr){ return modelptr == pic ; }), 1);
    BOOST_CHECK_EQUAL(desc_int.external_input_coupling.size(), 1);
    BOOST_CHECK_EQUAL(std::count_if(desc_int.external_input_coupling.begin(), desc_int.external_input_coupling.end(),
                                    [&pic](std::shared_ptr<model<Time>>& modelptr){ return modelptr == pic ; }), 1);
    BOOST_CHECK_EQUAL(desc_int.internal_coupling.size(), 1);
    BOOST_CHECK_EQUAL(std::count_if(desc_int.internal_coupling.begin(), desc_int.internal_coupling.end(),
                                    [&pg1, &pic](std::pair<std::shared_ptr<model<Time>>, std::shared_ptr<model<Time>>> & coupling){
                                            return coupling.first == pg1 &&
                                            coupling.second == pic ;})
                      , 1);
    BOOST_CHECK_EQUAL(desc_int.external_output_coupling.size(), 1);
    BOOST_CHECK_EQUAL(std::count_if(desc_int.external_output_coupling.begin(), desc_int.external_output_coupling.end(),
                                    [&pic](std::shared_ptr<model<Time>>& modelptr){ return modelptr == pic ; }), 1);

}
BOOST_AUTO_TEST_SUITE_END()

//same tests using the vector constructors
BOOST_AUTO_TEST_SUITE( p_coupled_using_vector_test_suite )
BOOST_AUTO_TEST_CASE( p_generator_into_coupled_vector_coupling_test )
{
    auto pg = make_atomic_ptr<generator<Time, Message>, Time>(Time{1});
    coupled<Time, Message> pc{
        std::vector<std::shared_ptr<model<Time>>>{pg},
        std::vector<std::shared_ptr<model<Time>>>{},
        std::vector<std::pair<std::shared_ptr<model<Time>>, std::shared_ptr<model<Time>>>>{},
        std::vector<std::shared_ptr<model<Time>>>{pg}
    };
    coupled<Time, Message>::description_type desc =  pc.get_description();
    BOOST_CHECK_EQUAL(desc.models.size(), 1);
    BOOST_CHECK_EQUAL(desc.models[0], pg);
    BOOST_CHECK_EQUAL(desc.external_input_coupling.size(), 0);
    BOOST_CHECK_EQUAL(desc.internal_coupling.size(), 0);
    BOOST_CHECK_EQUAL(desc.external_output_coupling.size(), 1);
    BOOST_CHECK_EQUAL(desc.external_output_coupling[0], pg);
}
BOOST_AUTO_TEST_CASE( p_generator_to_p_infinite_counter_into_coupled_vector_coupling_test )
{
    auto pg = make_atomic_ptr<generator<Time, Message>, Time>(Time{1});
    auto pic = make_atomic_ptr<infinite_counter<Time, Message>>();
    coupled<Time, Message> pc{
        std::vector<std::shared_ptr<model<Time>>>{pg, pic},
        std::vector<std::shared_ptr<model<Time>>>{pic},
        std::vector<std::pair<std::shared_ptr<model<Time>>, std::shared_ptr<model<Time>>>>{{pg, pic}},
        std::vector<std::shared_ptr<model<Time>>>{pic}
    };

    coupled<Time, Message>::description_type desc =  pc.get_description();
    BOOST_CHECK_EQUAL(desc.models.size(), 2);
    BOOST_CHECK_EQUAL(std::count_if(desc.models.begin(), desc.models.end(),
                                    [&pg](std::shared_ptr<model<Time>>& modelptr){ return modelptr == pg ; }), 1);
    BOOST_CHECK_EQUAL(std::count_if(desc.models.begin(), desc.models.end(),
                                    [&pic](std::shared_ptr<model<Time>>& modelptr){ return modelptr == pic ; }), 1);
    BOOST_CHECK_EQUAL(desc.external_input_coupling.size(), 1);
    BOOST_CHECK_EQUAL(std::count_if(desc.external_input_coupling.begin(), desc.external_input_coupling.end(),
                                    [&pic](std::shared_ptr<model<Time>>& modelptr){ return modelptr == pic ; }), 1);
    BOOST_CHECK_EQUAL(desc.internal_coupling.size(), 1);
    BOOST_CHECK_EQUAL(std::count_if(desc.internal_coupling.begin(), desc.internal_coupling.end(),
                                    [&pg, &pic](std::pair<std::shared_ptr<model<Time>>, std::shared_ptr<model<Time>>> & coupling){
                                        return coupling.first == pg && coupling.second == pic ;})
                      , 1);
    BOOST_CHECK_EQUAL(desc.external_output_coupling.size(), 1);
    BOOST_CHECK_EQUAL(std::count_if(desc.external_output_coupling.begin(), desc.external_output_coupling.end(),
                                    [&pic](std::shared_ptr<model<Time>>& modelptr){ return modelptr == pic ; }), 1);
}
BOOST_AUTO_TEST_CASE( p_generator_to_p_infinite_counter_into_coupleds_and_coupled_vector_coupling_test )
{
    auto pg1 = make_atomic_ptr<generator<Time, Message>, Time>(Time{1});
    auto pg2 = make_atomic_ptr<generator<Time, Message>, Time>(Time{1});
    auto pic = make_atomic_ptr<infinite_counter<Time, Message>>();
    auto pc1 = std::shared_ptr<coupled<Time, Message>>( new coupled<Time, Message>{
        std::vector<std::shared_ptr<model<Time>>>{pg1, pic},
        std::vector<std::shared_ptr<model<Time>>>{pic},
        std::vector<std::pair<std::shared_ptr<model<Time>>, std::shared_ptr<model<Time>>>>{{pg1, pic}},
        std::vector<std::shared_ptr<model<Time>>>{pic}
    });
    coupled<Time, Message> pc2{
        std::vector<std::shared_ptr<model<Time>>>{pg2, pc1},
        std::vector<std::shared_ptr<model<Time>>>{pc1},
        std::vector<std::pair<std::shared_ptr<model<Time>>, std::shared_ptr<model<Time>>>>{{pg2, pc1}},
        std::vector<std::shared_ptr<model<Time>>>{pc1}
    };

    //check pc2 model description
    coupled<Time, Message>::description_type desc =  pc2.get_description();
    BOOST_CHECK_EQUAL(desc.models.size(), 2);
    BOOST_CHECK_EQUAL(std::count_if(desc.models.begin(), desc.models.end(),
                                    [&pg2](std::shared_ptr<model<Time>>& modelptr){ return modelptr == pg2 ; }), 1);
    BOOST_CHECK_EQUAL(std::count_if(desc.models.begin(), desc.models.end(),
                                    [&pc1](std::shared_ptr<model<Time>>& modelptr){ return  modelptr == pc1; }), 1);
    BOOST_CHECK_EQUAL(desc.external_input_coupling.size(), 1);
    BOOST_CHECK_EQUAL(std::count_if(desc.external_input_coupling.begin(), desc.external_input_coupling.end(),
                                    [&pc1](std::shared_ptr<model<Time>>& modelptr){ return  modelptr == pc1; }), 1);
    BOOST_CHECK_EQUAL(desc.internal_coupling.size(), 1);
    BOOST_CHECK_EQUAL(std::count_if(desc.internal_coupling.begin(), desc.internal_coupling.end(),
                                    [&pg2, &pc1](std::pair<std::shared_ptr<model<Time>>, std::shared_ptr<model<Time>>> & coupling){
                                         return coupling.first == pg2 && coupling.second == pc1 ;})
                      , 1);
    BOOST_CHECK_EQUAL(desc.external_output_coupling.size(), 1);
    BOOST_CHECK_EQUAL(std::count_if(desc.external_output_coupling.begin(), desc.external_output_coupling.end(),
                                    [&pc1](std::shared_ptr<model<Time>>& modelptr){ return  modelptr == pc1; }), 1);
    //check pc1 model description
    coupled<Time, Message>::description_type desc_int = std::dynamic_pointer_cast<coupled<Time, Message>>(desc.models[1])->get_description();
    BOOST_CHECK_EQUAL(desc_int.models.size(), 2);
    BOOST_CHECK_EQUAL(std::count_if(desc_int.models.begin(), desc_int.models.end(),
                                    [&pg1](std::shared_ptr<model<Time>>& modelptr){ return modelptr == pg1 ; }), 1);
    BOOST_CHECK_EQUAL(std::count_if(desc_int.models.begin(), desc_int.models.end(),
                                    [&pic](std::shared_ptr<model<Time>>& modelptr){ return modelptr == pic ; }), 1);
    BOOST_CHECK_EQUAL(desc_int.external_input_coupling.size(), 1);
    BOOST_CHECK_EQUAL(std::count_if(desc_int.external_input_coupling.begin(), desc_int.external_input_coupling.end(),
                                    [&pic](std::shared_ptr<model<Time>>& modelptr){ return modelptr == pic ; }), 1);
    BOOST_CHECK_EQUAL(desc_int.internal_coupling.size(), 1);
    BOOST_CHECK_EQUAL(std::count_if(desc_int.internal_coupling.begin(), desc_int.internal_coupling.end(),
                                    [&pg1, &pic](std::pair<std::shared_ptr<model<Time>>, std::shared_ptr<model<Time>>> & coupling){
                                        return coupling.first == pg1 && coupling.second == pic;})
                      , 1);
    BOOST_CHECK_EQUAL(desc_int.external_output_coupling.size(), 1);
    BOOST_CHECK_EQUAL(std::count_if(desc_int.external_output_coupling.begin(), desc_int.external_output_coupling.end(),
                                    [&pic](std::shared_ptr<model<Time>>& modelptr){ return modelptr == pic ; }), 1);

}

BOOST_AUTO_TEST_SUITE_END()




BOOST_AUTO_TEST_SUITE_END()
