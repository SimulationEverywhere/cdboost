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
#include <boost/simulation/pdevs/coordinator.hpp>
#include <boost/simulation/pdevs/basic_models/infinite_counter.hpp>
#include <boost/simulation/pdevs/basic_models/generator.hpp>
#include <boost/simulation/pdevs/basic_models/istream.hpp>
#include <math.h>
#include <boost/simulation/convenience.hpp>


using namespace boost::simulation;
using namespace boost::simulation::pdevs;
using namespace boost::simulation::pdevs::basic_models;
using namespace std;

using Time=double;
using Message=boost::any;

/**
  This test suite uses simulators and basic models that were tested in other suites before
  to check the models are handled correctly by the simulator.
  */


BOOST_AUTO_TEST_SUITE( p_simulator_test_suite )
//pgenerartor based tests
BOOST_AUTO_TEST_SUITE( p_simulated_generator_test_suite )
BOOST_AUTO_TEST_CASE( simulated_generator_init_test )
{
    //Create Simulator for the PGenerator model
    //Send init message to the simulator
    //check the time advance is the initial time is the expected one.
    {
        auto pa = make_atomic_ptr<generator<Time, Message>, Time>(Time{1});
        coordinator<Time, Message> s{pa};
        BOOST_CHECK_EQUAL( s.init(Time(0)), Time{1});
    }
    {
        auto pa = make_atomic_ptr<generator<Time, Message>, Time>(Time{1});
        coordinator<Time, Message> s{pa};
        BOOST_CHECK_EQUAL( s.init(Time(3)), Time{4});
    }

}
BOOST_AUTO_TEST_CASE( simulated_generator_in_coupled_init_test )
{
    //Create Simulator for the PGenerator model
    //Send init message to the simulator
    //check the time advance is the initial time is the expected one.
    {
        auto pa = make_atomic_ptr<generator<Time, Message>, Time>(Time{1});
        auto c = std::shared_ptr<coupled<Time, Message>>(new coupled<Time, Message>{{pa}, {}, {}, {pa}});

        coordinator<Time, Message> s{c};
        BOOST_CHECK_EQUAL( s.init(Time(0)), Time{1});
    }
    {
        auto pa = make_atomic_ptr<generator<Time, Message>, Time>(Time{1});
        coordinator<Time, Message> s{pa};
        BOOST_CHECK_EQUAL( s.init(Time(3)), Time{4});
    }

}
BOOST_AUTO_TEST_SUITE_END()
//pinfinite_counter based tests
BOOST_AUTO_TEST_SUITE( p_simulated_infinite_counter_test_suite )
BOOST_AUTO_TEST_CASE( simulated_infinite_counter_init_test )
{
    //Create Simulator for the PInfiniteCounter model
    //Send init message to the simulator
    //check the time advance is the initial time is infinity
    {
        auto pa = make_atomic_ptr<infinite_counter<Time, Message>>();
        coordinator<Time, Message> s{pa};
        BOOST_CHECK( isinf(s.init(Time(0))));
    }
}
BOOST_AUTO_TEST_CASE( simulated_infinite_counter_advance_external_internal_test )
{
    //Create Simulator for the PInfiniteCounter model
    //Send init message to the simulator
    //check the time advance is the initial time is infinity
    {
        auto pa = make_atomic_ptr<infinite_counter<Time, Message>>();
        std::shared_ptr<std::istringstream> piss{ new std::istringstream{} };
        piss->str("1 1 \n 1 2 \n 1 3 \n 1 4 \n 2 5 \n 2 6 \n 2 7 \n 2 8 \n 2 0 ");
        auto pf = make_atomic_ptr<istream<Time, Message, int, int>>(piss, Time(0));

        auto pc = std::shared_ptr<coupled<Time, Message>>(new coupled<Time, Message>{{pf, pa}, {}, {{pf, pa}}, {pa}});

//        std::cout << "models:  pf->" << pf.get() << " pa->" << pa.get() << " pc->" << pc.get() << std::endl;


        coordinator<Time, Message> s{pc};
        Time t = s.init(Time{0});
        BOOST_CHECK_EQUAL( t, Time{1} );
//at 1
        auto reply = s.collectOutputs(t);
        s.advanceSimulation( Time{1});
        BOOST_REQUIRE_EQUAL(reply.size(), 0);
        BOOST_CHECK_EQUAL(s.next(), Time{2});
//at 2
        reply = s.collectOutputs(t);
        s.advanceSimulation( Time{2});
        BOOST_REQUIRE_EQUAL(reply.size(), 0);
        BOOST_CHECK_EQUAL(s.next(), Time{2});
//at 2 + 0
        reply = s.collectOutputs( Time{2});
        s.advanceSimulation( Time{2});
        BOOST_REQUIRE_EQUAL(reply.size(), 1);
        BOOST_CHECK_EQUAL(boost::any_cast<int>(reply[0]), 8);
        BOOST_CHECK(isinf(s.next()));
    }
}
BOOST_AUTO_TEST_CASE( simulated_infinite_counter_advance_confluenced_test )
{
    //Create Simulator for the PInfiniteCounter model
    //Send init message to the simulator
    //check the time advance is the initial time is infinity
    {

        auto pa = make_atomic_ptr<infinite_counter<Time, Message>>();
        std::shared_ptr<std::istringstream> piss{ new std::istringstream{} };
        piss->str("1 1 \n 1 2 \n 1 3 \n 1 4 \n 1 0 \n 2 5 \n 2 6 \n 2 7 \n 2 8 \n 2 0 ");
        auto pf = make_atomic_ptr<istream<Time, Message, int, int>, std::shared_ptr<std::istringstream>, Time>(piss, Time(0));

        auto pc = std::shared_ptr<coupled<Time, Message>>(new coupled<Time, Message>{{pf, pa}, {}, {{pf, pa}}, {pa}});

        coordinator<Time, Message> s{pc};

        Time t = s.init(Time{0});
        BOOST_CHECK_EQUAL( t, Time{1} );
        auto reply =  s.collectOutputs(t);
        BOOST_REQUIRE_EQUAL(reply.size(), 0);
        BOOST_CHECK_EQUAL(s.next(), Time{1});

        s.advanceSimulation( Time{1});
        BOOST_CHECK_EQUAL(s.next(), Time{1});

        reply = s.collectOutputs(Time{1});
        BOOST_REQUIRE_EQUAL(reply.size(), 1);
        BOOST_CHECK_EQUAL(boost::any_cast<int>(reply[0]), 4);

        s.advanceSimulation( Time{1});
        BOOST_CHECK_EQUAL(s.next(), Time{2});

        reply = s.collectOutputs(Time{2});
        BOOST_REQUIRE_EQUAL(reply.size(), 0);

        s.advanceSimulation( Time{2});
        BOOST_CHECK_EQUAL(s.next(), Time{2});

        reply = s.collectOutputs(Time{2});
        BOOST_REQUIRE_EQUAL(reply.size(), 1);
        BOOST_CHECK_EQUAL(boost::any_cast<int>(reply[0]), 4);

        s.advanceSimulation( Time{2});
        BOOST_CHECK(isinf(s.next()));
    }
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
