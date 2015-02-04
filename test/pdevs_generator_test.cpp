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
#include <boost/simulation/pdevs/basic_models/generator.hpp>
#include <boost/rational.hpp>
#include <boost/any.hpp>

using namespace boost::simulation;
using namespace boost::simulation::pdevs;
using namespace boost::simulation::pdevs::basic_models;
using namespace std;

using Time=boost::rational<int>;
using Message=boost::any;

BOOST_AUTO_TEST_SUITE( p_generator_test_suite )
BOOST_AUTO_TEST_CASE( p_generator_init_test )
{
    //Create a generator with step 1 and obtain the ta=1;
    generator<Time, Message> a(Time{1});
    BOOST_CHECK_EQUAL( a.advance(), Time{1});
}

BOOST_AUTO_TEST_CASE( p_generator_tick_test )
{
    //Create different step generators
    //check that advance always match the step after internal transitions
    for (int i=1; i < 100; i++){
        Time t = Time(i);
        for (int j=1; j < 100; j++){
            generator<Time, Message> a(t);
            BOOST_CHECK_EQUAL(boost::any_cast<int>(a.out()[0]), 1);
            a.internal();
            BOOST_CHECK_EQUAL( a.advance(),t);
        }
    }
}

BOOST_AUTO_TEST_CASE( p_generator_output_value_test )
{
    //Create a generator with  a custom out value
    //Check the output is always the preset value
    Time t(1);
    generator<Time, Message> a(t, 5);
    for (int j=1; j < 100; j++){
        BOOST_CHECK_EQUAL(boost::any_cast<int>(a.out()[0]), 5);
        a.internal();
        BOOST_CHECK_EQUAL( a.advance(),t);
    }
}

BOOST_AUTO_TEST_SUITE_END()

