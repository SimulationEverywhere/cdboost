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
#include <boost/simulation/pdevs/runner.hpp>
#include <boost/simulation/pdevs/basic_models/generator.hpp>
#include <boost/simulation/pdevs/basic_models/istream.hpp>
#include <math.h>

using namespace boost::simulation;
using namespace boost::simulation::pdevs;
using namespace boost::simulation::pdevs::basic_models;
using namespace std;

using Time=double;
using Message=boost::any;

BOOST_AUTO_TEST_SUITE( p_runner_for_pdevs_test_suite )

BOOST_AUTO_TEST_SUITE( silent_p_runner_test_suite )
BOOST_AUTO_TEST_CASE( p_runner_runs_generator_until_10sec_test )
{
    //create a generator with tick 1, embed it in a coupled and create the p_runner
    //run until 10, when it ends, check end time is 10.
    shared_ptr<pdevs::atomic<Time, Message>> pa{ new generator<Time, Message>{Time{1}} };
    shared_ptr<coupled<Time, Message>> cm( new coupled<Time, Message>{{pa}, {}, {}, {pa}});
    runner<Time, Message> r(cm, Time{0});
    BOOST_CHECK_EQUAL( r.runUntil(Time{10}), Time{10});
}


BOOST_AUTO_TEST_CASE( p_runner_stops_on_passive_test )
{
    //create a fixed model with events below 20
    //put it in a coupled model
    //run until 20
    //check that run ends in infinity
    {
        shared_ptr<istringstream> piss{ new istringstream{} };
        piss->str("1 1 \n 4 4 \n 5 5 \n 6 6 \n 8 8 \n 9 9 ");
        shared_ptr<pdevs::atomic<Time, Message>> pf{ new istream<Time, Message, int, int>{piss, Time{0}}};

        shared_ptr<coupled<Time, Message>> cm( new coupled<Time, Message>{{pf}, {}, {}, {pf}});
        runner<Time, Message> r(cm, Time{0});

        BOOST_CHECK( isinf(r.runUntil(Time{20})));
    }
    //repeat to stop in middle of the fixed events list
    //check it ends in last event before limit and returns the next
    {
        shared_ptr<istringstream> piss{ new istringstream{} };
        piss->str("1 1 \n 4 4 \n 5 5 \n 6 6 \n 8 8 \n 9 9 ");
        shared_ptr<pdevs::atomic<Time, Message>> pf{ new istream<Time, Message, int, int>{piss, Time{0}}};

        shared_ptr<coupled<Time, Message>> cm( new coupled<Time, Message>{{pf}, {}, {}, {pf}});
        runner<Time, Message> r(cm, Time{0});

        BOOST_CHECK_EQUAL( r.runUntil(Time{7}), Time{8});
    }
}

BOOST_AUTO_TEST_CASE( p_runner_until_passive_test )
{
    //create a fixed model with events below 10
    //run until passivate
    //check that run doesn't throw
    {
        shared_ptr<istringstream> piss{ new istringstream{} };
        piss->str("1 1 \n 4 4 \n 5 5 \n 6 6 \n 8 8 \n 9 9 ");
        shared_ptr<pdevs::atomic<Time, Message>> pf{ new istream<Time, Message, int, int>{piss, Time{0}}};

        shared_ptr<coupled<Time, Message>> cm( new coupled<Time, Message>{{pf}, {}, {}, {pf}});
        runner<Time, Message> r(cm, Time{0});
        BOOST_CHECK_NO_THROW( r.runUntilPassivate());
    }
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( p_runner_with_output_test_suite )

BOOST_AUTO_TEST_CASE( p_runner_runs_generator_until_10sec_test )
{
    //create a generator with tick 1, embed it in a coupled and create the p_runner
    //run until 10, when it ends, check the output.
    shared_ptr<pdevs::atomic<Time, Message>> pa{ new generator<Time, Message>{Time{1}} };
    shared_ptr<coupled<Time, Message>> cm( new coupled<Time, Message>{{pa}, {}, {}, {pa}});

    ostringstream oss;
    runner<Time, Message> r(cm, Time{0}, oss, [](ostream& os, boost::any m){ os << boost::any_cast<int>(m);});

    r.runUntil(Time{10});

    string s_out = oss.str();

    if (is_same<Time, boost::rational<int>>()) { //Time output differs
        BOOST_CHECK_EQUAL( s_out,
                           "1/1 1\n"
                           "2/1 1\n"
                           "3/1 1\n"
                           "4/1 1\n"
                           "5/1 1\n"
                           "6/1 1\n"
                           "7/1 1\n"
                           "8/1 1\n"
                           "9/1 1\n"
                           );
    } else if (is_same<Time, float>()){
        BOOST_CHECK_EQUAL( s_out,
                           "1 1\n2 1\n3 1\n4 1\n5 1\n6 1\n7 1\n8 1\n9 1\n"
                           );
    } else if (is_same<Time, double>()){
        BOOST_CHECK_EQUAL( s_out,
                           "1 1\n2 1\n3 1\n4 1\n5 1\n6 1\n7 1\n8 1\n9 1\n"
                           );
    } else {
        BOOST_CHECK(false); //Don't know how to handle the Time output
    }
}

BOOST_AUTO_TEST_CASE( p_runner_runs_generator_until_passivate_test )
{
    //create a fixed input
    //run until passivate and check the output.
    shared_ptr<istringstream> piss{ new istringstream{} };
    piss->str("1 1 \n 4 4 \n 5 5 \n 6 6 \n 8 8 \n 9 9 ");
    shared_ptr<pdevs::atomic<Time, Message>> pf{ new istream<Time, Message, int, int>{piss, Time{0}}};

    shared_ptr<coupled<Time, Message>> cm( new coupled<Time, Message>{{pf}, {}, {}, {pf}});

    ostringstream oss;
    runner<Time, Message> r(cm, Time{0}, oss, [](ostream& os, boost::any m){ os << boost::any_cast<int>(m);});

    r.runUntilPassivate();

    string s_out = oss.str();

    if (is_same<Time, boost::rational<int>>()) { //Time output differs
        BOOST_CHECK_EQUAL( s_out,
                           "1/1 1\n"
                           "4/1 4\n"
                           "5/1 5\n"
                           "6/1 6\n"
                           "8/1 8\n"
                           "9/1 9\n"
                           );
    } else if (is_same<Time, float>()){
        BOOST_CHECK_EQUAL( s_out,
                           "1 1\n4 4\n5 5\n6 6\n8 8\n9 9\n"
                           );
    } else if (is_same<Time, double>()){
        BOOST_CHECK_EQUAL( s_out,
                           "1 1\n4 4\n5 5\n6 6\n8 8\n9 9\n"
                           );
    } else {
        BOOST_CHECK(false); //Don't know how to handle the Time output
    }
}
BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE_END()
