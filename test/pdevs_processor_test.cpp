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
#include <algorithm>
#include <boost/rational.hpp>
#include <boost/simulation/pdevs/basic_models/processor.hpp>
#include <boost/simulation/pdevs/coordinator.hpp>
#include <math.h>

using namespace boost::simulation;
using namespace boost::simulation::pdevs;
using namespace boost::simulation::pdevs::basic_models;

using Time=double;
using Message=int;

BOOST_AUTO_TEST_SUITE( processor_test_suite )
BOOST_AUTO_TEST_CASE( processing_single_jobs_test )
{
    //create processors with different delays
    //check passive on wait
    //input 1 to processor
    //check the ta is the predefined
    //run internal
    //check ta=passive and out=job was input
    for (int i=0; i<10; i++){
        processor<Time, Message> p{static_cast<Time>(i)};
        BOOST_CHECK( isinf(p.advance()) );
        p.external({i}, Time{1});
        BOOST_CHECK_EQUAL(p.advance(), Time(i));
        BOOST_CHECK_EQUAL(boost::any_cast<int>(p.out()[0]), i);
        p.internal();
        BOOST_CHECK( isinf(p.advance()));
    }
}
BOOST_AUTO_TEST_CASE( processing_sequential_jobs_test )
{
    //create processors with different delays
    //check passive on wait
    //input to processor
    //check the ta is the predefined
    //run internal and input to procesor
    //check ta=passive and out=job was input after each internal
    //check passive at the end of last job
    for (int i{0}; i<10; i++){
        processor<Time, Message> p{static_cast<Time>(i)};
        BOOST_CHECK( isinf(p.advance()) );

        for (int j{0}; j <= i ; j++){
            p.external({j}, Time{1});
            BOOST_CHECK_EQUAL(p.advance(), Time(i));
            BOOST_CHECK_EQUAL(boost::any_cast<int>(p.out()[0]), j);
            p.internal();
            BOOST_CHECK( isinf(p.advance()));
        }
    }
}
BOOST_AUTO_TEST_CASE( multiple_jobs_return_one_at_the_time_test )
{
    //create processor.
    //input multiple jobs
    //obtain n separated jobs
    processor<Time, Message> p{Time{1}};
    BOOST_CHECK( isinf(p.advance()) );

    p.external({1, 2, 3, 4}, Time{0});
    for (int i=0; i<4; i++){
        BOOST_CHECK_EQUAL(p.advance(), Time{1});
        BOOST_CHECK_EQUAL(p.out().size(), 1);
        p.internal();
    }
    BOOST_CHECK( isinf(p.advance()));

}
BOOST_AUTO_TEST_SUITE_END()

