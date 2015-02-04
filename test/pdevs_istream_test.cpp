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
#include <string>
#include <iostream>
#include <sstream>
#include <boost/test/unit_test.hpp>
#include <boost/any.hpp>
#include <boost/simulation/pdevs/basic_models/istream.hpp>
#include <math.h>

using namespace boost::simulation;
using namespace boost::simulation::pdevs;
using namespace boost::simulation::pdevs::basic_models;
using namespace std;

using Time=double;
using Message=boost::any;

BOOST_AUTO_TEST_SUITE( pistream_test_suite )
BOOST_AUTO_TEST_CASE( pistream_simple_of_single_events_test )
{
    //Create a istream with events every second outputing [1..10]
    //Check that outputs the 10 numbers every 1 second and last one is infinity

    shared_ptr<istringstream> piss{ new istringstream{} };
    piss->str("0 0");
    //init
    istream<Time, Message, int, int> pf{piss, Time(0)};
    BOOST_CHECK_EQUAL(pf.advance(), Time(0));

    // only message
    BOOST_REQUIRE_EQUAL(pf.out().size(), 1);
    BOOST_CHECK_EQUAL(boost::any_cast<int>(pf.out()[0]), 0);
    pf.internal();
    BOOST_CHECK( isinf(pf.advance()));
}

BOOST_AUTO_TEST_CASE( pistream_simple_of_multiple_events_test )
{
    //Create a istream with events every second outputing [1..10]
    //Check that outputs the 10 numbers every 1 second and last one is infinity

    shared_ptr<istringstream> piss{ new istringstream{} };
    piss->str("0 0 \n 0 1 \n 0 2 ");
    //init
    istream<Time, Message, int, int> pf{piss, Time(0)};
    BOOST_CHECK_EQUAL(pf.advance(), Time(0));

    // only output
    BOOST_REQUIRE_EQUAL(pf.out().size(), 3);
    pf.internal();
    BOOST_CHECK( isinf(pf.advance()));
}


BOOST_AUTO_TEST_CASE( pistream_as_generator_of_single_events_test )
{
    //Create a istream with events every second outputing [1..10]
    //Check that outputs the 10 numbers every 1 second and last one is infinity

    shared_ptr<istringstream> piss{ new istringstream{} };
    piss->str("0 0 \n 1 1 \n 2 2 \n 3 3 \n 4 4 \n 5 5 \n 6 6 \n 7 7 \n 8 8 \n 9 9 \n 10 10");
    //init
    istream<Time, Message, int, int> pf{piss, Time(0)};
    BOOST_CHECK_EQUAL(pf.advance(), Time(0));

    //consume
    for (int i=0; i < 10 ; i++){
        BOOST_REQUIRE_EQUAL(pf.out().size(), 1);
        BOOST_CHECK_EQUAL(boost::any_cast<int>(pf.out()[0]), i);
        pf.internal();
        BOOST_CHECK_EQUAL(pf.advance(), Time(1));
    }
    //last message
    BOOST_REQUIRE_EQUAL(pf.out().size(), 1);
    BOOST_CHECK_EQUAL(boost::any_cast<int>(pf.out()[0]), 10);
    pf.internal();
    BOOST_CHECK( isinf(pf.advance()));
}


BOOST_AUTO_TEST_CASE( pistream_as_generator_of_multiple_events_test )
{
    //Create a istream with events every second outputing [1..10]
    //Check that outputs the 10 numbers every 1 second and last one is infinity

    shared_ptr<istringstream> piss{ new istringstream{} };
    piss->str("1 1 \n 1 1 \n 2 2 \n 2 2 \n 3 3 \n 3 3 \n 4 4 \n 4 4 \n 5 5 \n 5 5");
    //init
    istream<Time, Message, int, int> pf{piss, Time(0)};
    BOOST_CHECK_EQUAL(pf.advance(), Time(1));
    //advance simulation
    for (int i=1; i < 5; i++){
        BOOST_REQUIRE_EQUAL(pf.out().size(), 2);
        BOOST_CHECK_EQUAL(boost::any_cast<int>(pf.out()[0]), i);
        BOOST_CHECK_EQUAL(boost::any_cast<int>(pf.out()[1]), i);
        pf.internal();
        BOOST_CHECK_EQUAL(pf.advance(), Time(1));
    }
    //last item
    BOOST_REQUIRE_EQUAL(pf.out().size(), 2);
    BOOST_CHECK_EQUAL(boost::any_cast<int>(pf.out()[0]), 5);
    BOOST_CHECK_EQUAL(boost::any_cast<int>(pf.out()[1]), 5);
    pf.internal();
    BOOST_CHECK( isinf(pf.advance()));
}

//custom processor of input
BOOST_AUTO_TEST_CASE( pistream_with_custom_processor_as_generator_of_multiple_events_test )
{
    //Create a istream with events every second outputing [1..10]
    //Check that outputs the 10 numbers every 1 second and last one is infinity

    shared_ptr<istringstream> piss{ new istringstream{} };
    piss->str("1 hello \n 1 world \n 2 hello \n 2 world");
    //init
    istream<Time, Message, int, int> pf{piss, Time(0),
                [](const string& s, Time& t_next, boost::any& m_next)->void{
            //intermediary vars for casting
            int tmp_next;
            string tmp_next_out;
            std::stringstream ss;
            ss.str(s);
            ss >> tmp_next;
            t_next = static_cast<Time>(tmp_next);
            ss >> tmp_next_out;
            m_next = static_cast<boost::any>(tmp_next_out);
            std::string thrash;
            ss >> thrash;
            if ( 0 != thrash.size()) throw std::exception();
        }};

    BOOST_CHECK_EQUAL(pf.advance(), Time(1));
    //advance simulation
    BOOST_REQUIRE_EQUAL(pf.out().size(), 2);
    BOOST_CHECK(any_of(pf.out().begin(), pf.out().end(), [](const boost::any& m ){ string s = boost::any_cast<string>(m); return s.compare("hello")==0;}));
    BOOST_CHECK(any_of(pf.out().begin(), pf.out().end(), [](const boost::any& m ){ string s = boost::any_cast<string>(m); return s.compare("world")==0;}));
    pf.internal();
    BOOST_CHECK_EQUAL(pf.advance(), Time(1));
    //last item
    BOOST_REQUIRE_EQUAL(pf.out().size(), 2);
    BOOST_CHECK(any_of(pf.out().begin(), pf.out().end(), [](const boost::any& m ){ string s = boost::any_cast<string>(m); return s.compare("hello")==0;}));
    BOOST_CHECK(any_of(pf.out().begin(), pf.out().end(), [](const boost::any& m ){ string s = boost::any_cast<string>(m); return s.compare("world")==0;}));
    pf.internal();
    BOOST_CHECK( isinf(pf.advance()));
}





BOOST_AUTO_TEST_SUITE_END()

