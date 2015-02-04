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
#include <boost/mpl/vector.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/quote.hpp>
#include <boost/mpl/protect.hpp>
#include <boost/mpl/bind.hpp>
#include <boost/mpl/list.hpp>


#include <boost/simulation/pdevs/coupled.hpp>
#include <boost/simulation/pdevs/coordinator.hpp>
#include <boost/simulation/pdevs/basic_models/generator.hpp>
#include <boost/simulation/pdevs/basic_models/infinite_counter.hpp>
#include <boost/simulation/pdevs/basic_models/processor.hpp>
#include <boost/simulation/pdevs/basic_models/istream.hpp>
#include <boost/simulation/convenience.hpp>



using namespace boost::simulation;
using namespace boost::simulation::pdevs;
using namespace boost::simulation::pdevs::basic_models;

using Time=double;
using Message=boost::any;

template<class TIME=Time, class MSG=Message>
class ConfluenceTestHelper : public processor<TIME, MSG>{
public:
    ConfluenceTestHelper(TIME t) : processor<TIME, MSG>(t){}
    void confluence(const std::vector<MSG>& mb, const TIME& t)  noexcept {
        BOOST_CHECK(false); //, "This function should not be called by the test"); //in confluence now
    }
};

//types to be used by tests in vectors of <TIME, MSG, FEL>
//using test_types=boost::mpl::list<boost::mpl::vector<int, double, boost::mpl::quote2<std::pair>>,boost::mpl::vector<double, double, boost::mpl::quote2<std::pair>>>;
using test_types=boost::mpl::list<
    boost::mpl::vector<Time, boost::any, boost::mpl::quote2<nullqueue>>,
    boost::mpl::vector<Time, int, boost::mpl::quote2<nullqueue>>
>;

/**
  This test suite uses simulators and basic models that were tested in other suites before
  The time for "next" in coordinators and simulators is absolute, starting at the time set
  by initializer.
  */

BOOST_AUTO_TEST_SUITE( p_coordinator_using_priority_queue_vector_test_suite )
BOOST_AUTO_TEST_SUITE( p_coordinated_generator_test_suite )
//generators
BOOST_AUTO_TEST_CASE_TEMPLATE( p_coordinated_generator_produces_right_output_test, T, test_types){
    //obtaining template parameters for test
    using TIME=typename boost::mpl::at<T, boost::mpl::int_<0>>::type;
    using MSG=typename boost::mpl::at<T, boost::mpl::int_<1>>::type;
    using FELAUX=typename boost::mpl::at<T, boost::mpl::int_<2>>::type;

    //create a generator into a coupled model.
    //connect only its output
    //check the right output is generated when advancing.
    std::shared_ptr<atomic<TIME, MSG>> pa{ new generator<TIME, MSG>{TIME{1}, 2}};
    auto cm = std::shared_ptr<coupled<TIME, MSG>>(new coupled<TIME, MSG>{{pa}, {}, {}, {pa}});
    auto c = std::shared_ptr<coordinator<TIME, MSG, priority_queue_vector>>( new coordinator<TIME, MSG, priority_queue_vector>(cm));

    TIME t = c->init(TIME{0});
    BOOST_CHECK_EQUAL( t, TIME{1} ); //first advance
    auto reply = c->collectOutputs(t);
    c->advanceSimulation( TIME{1});
    t = c->next();
    BOOST_CHECK_EQUAL( t, TIME(2)); //after input half the tick passed
    BOOST_REQUIRE_EQUAL( reply.size(), 1);
    BOOST_CHECK_EQUAL( boost::any_cast<int>(reply[0]), 2);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( p_coordinated__multiple_generators_produces_right_output_test, T, test_types )
{
    //obtaining template parameters for test
    using TIME=typename boost::mpl::at<T, boost::mpl::int_<0>>::type;
    using MSG=typename boost::mpl::at<T, boost::mpl::int_<1>>::type;
    using FELAUX=typename boost::mpl::at<T, boost::mpl::int_<2>>::type;


    //create 3 generator into 3 coupled models in cascade.
    //connect only its output
    //check the time advanced until input was consumed

    //first model
    std::shared_ptr<atomic<TIME, MSG>> pa1{ new generator<TIME, MSG>{TIME{1}, 1}};
    auto cm1 = std::shared_ptr<coupled<TIME, MSG>>(new coupled<TIME, MSG>{{pa1}, {}, {}, {pa1}});
    //second model
    std::shared_ptr<atomic<TIME, MSG>> pa2{ new generator<TIME, MSG>{TIME{2}, 2}};
    auto cm2 = std::shared_ptr<coupled<TIME, MSG>>(new coupled<TIME, MSG>{{pa2, cm1}, {}, {}, {pa2, cm1}});
    //third model
    std::shared_ptr<atomic<TIME, MSG>> pa3{ new generator<TIME, MSG>{TIME{3}, 3}};
    auto cm3 = std::shared_ptr<coupled<TIME, MSG>>(new coupled<TIME, MSG>{{pa3, cm2}, {}, {}, {pa3, cm2}});
    //coordination
    auto c = std::shared_ptr<coordinator<TIME, MSG, priority_queue_vector>>( new coordinator<TIME, MSG, priority_queue_vector>(cm3));

    TIME t = c->init(TIME{0});
    BOOST_CHECK_EQUAL( t, TIME{1} ); //first advance
    //at time 1
    auto reply = c->collectOutputs(t);
    c->advanceSimulation( TIME{1});
    t = c->next();
    BOOST_CHECK_EQUAL( t, TIME{2}); //after input half the tick passed
    BOOST_REQUIRE_EQUAL( reply.size(), 1);
    BOOST_CHECK_EQUAL( boost::any_cast<int>(reply[0]), 1);
    //at time 2
    reply = c->collectOutputs(t);
    c->advanceSimulation( TIME{2});
    t = c->next();
    BOOST_CHECK_EQUAL( t, TIME{3}); //after input half the tick passed
    BOOST_REQUIRE_EQUAL( reply.size(), 2);
    BOOST_CHECK_EQUAL( std::count_if(reply.begin(), reply.end(), [](MSG& m) { return boost::any_cast<int>(m) ==  1;}), 1);
    BOOST_CHECK_EQUAL( std::count_if(reply.begin(), reply.end(), [](MSG& m) { return boost::any_cast<int>(m) ==  2;}), 1);
    //at time 3

    reply = c->collectOutputs(t);
    c->advanceSimulation( TIME{3});
    t = c->next();
    BOOST_CHECK_EQUAL( t, TIME{4}); //after input half the tick passed
    BOOST_REQUIRE_EQUAL( reply.size(), 2);
    BOOST_CHECK_EQUAL( std::count_if(reply.begin(), reply.end(), [](MSG& m) { return boost::any_cast<int>(m) ==  1;}), 1);
    BOOST_CHECK_EQUAL( std::count_if(reply.begin(), reply.end(), [](MSG& m) { return boost::any_cast<int>(m) ==  3;}), 1);
    //at time 4

    reply = c->collectOutputs(t);
    c->advanceSimulation( TIME{4});
    t = c->next();
    BOOST_CHECK_EQUAL( t, TIME{5}); //after input half the tick passed
    BOOST_REQUIRE_EQUAL( reply.size(), 2);
    BOOST_CHECK_EQUAL( std::count_if(reply.begin(), reply.end(), [](MSG& m) { return boost::any_cast<int>(m) ==  1;}), 1);
    BOOST_CHECK_EQUAL( std::count_if(reply.begin(), reply.end(), [](MSG& m) { return boost::any_cast<int>(m) ==  2;}), 1);
    //at time 5

    reply = c->collectOutputs(t);
    c->advanceSimulation( TIME{5});
    t = c->next();
    BOOST_CHECK_EQUAL( t, TIME{6}); //after input half the tick passed
    BOOST_REQUIRE_EQUAL( reply.size(), 1);
    BOOST_CHECK_EQUAL( std::count_if(reply.begin(), reply.end(), [](MSG& m) { return boost::any_cast<int>(m) ==  1;}), 1);
    //at time 6

    reply = c->collectOutputs(t);
    c->advanceSimulation( TIME{6});
    t = c->next();
    BOOST_CHECK_EQUAL( t, TIME{7}); //after input half the tick passed
    BOOST_REQUIRE_EQUAL( reply.size(), 3);
    BOOST_CHECK_EQUAL( std::count_if(reply.begin(), reply.end(), [](MSG& m) { return boost::any_cast<int>(m) ==  1;}), 1);
    BOOST_CHECK_EQUAL( std::count_if(reply.begin(), reply.end(), [](MSG& m) { return boost::any_cast<int>(m) ==  2;}), 1);
    BOOST_CHECK_EQUAL( std::count_if(reply.begin(), reply.end(), [](MSG& m) { return boost::any_cast<int>(m) ==  3;}), 1);

}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( pcoordinated_generators_and_infinite_counters_test_suite )
//generators and infinite_counters
BOOST_AUTO_TEST_CASE( generator_send_to_infinite_counter_manual_reset_test )
{
    //create a generator and a infinite_counter
    std::shared_ptr<atomic<Time, Message>> pg{ new generator<Time, Message>{Time{2}, 1} };
    std::shared_ptr<atomic<Time, Message>> pic{ new infinite_counter<Time, Message>{} };

    //input
    std::shared_ptr<std::istringstream> piss{ new std::istringstream{} };
    piss->str(" 3 0 ");
    auto pf = make_atomic_ptr<istream<Time, Message, int, int>, std::shared_ptr<std::istringstream>, Time>(piss, Time(0));

    //couple them
    auto cm = std::shared_ptr<coupled<Time, Message>>(new coupled<Time, Message>{{pg, pic, pf}, {}, {{pg, pic}, {pf, pic}}, {pic}});
    //coordinate
    auto c = std::shared_ptr<coordinator<Time, boost::any, priority_queue_vector>>( new coordinator<Time, boost::any, priority_queue_vector>(cm));
    Time t = c->init(Time{0});
    BOOST_CHECK_EQUAL( t, Time{2} ); //first advance

    //at time 2
    auto reply = c->collectOutputs(t);
    c->advanceSimulation( Time{2});
    t = c->next();
    BOOST_CHECK_EQUAL( t, Time{3}); //input from pf
    BOOST_REQUIRE_EQUAL( reply.size(), 0);

    //at time 3
    reply = c->collectOutputs(t);
    c->advanceSimulation( Time{3});
    t = c->next();
    BOOST_CHECK_EQUAL( t, Time{3}); //asking for count

    //again at 3
    reply = c->collectOutputs(t);
    c->advanceSimulation( Time{3});
    t = c->next();
    BOOST_CHECK_EQUAL( t, Time{4}); //next generator tick
    BOOST_REQUIRE_EQUAL( reply.size(), 1);
    BOOST_CHECK_EQUAL( boost::any_cast<int>(reply[0]), 1);
}
BOOST_AUTO_TEST_CASE( generators_send_to_infinite_counter_test )
{
    //create a generator and a infinite_counter
    std::shared_ptr<atomic<Time, Message>> pg1{ new generator<Time, Message>{Time{1}, 1} };
    std::shared_ptr<atomic<Time, Message>> pg2{ new generator<Time, Message>{Time{2}, 0} };
    std::shared_ptr<atomic<Time, Message>> pic{ new infinite_counter<Time, Message>{} };

    //couple them
    auto cm = std::shared_ptr<coupled<Time, Message>>(new coupled<Time, Message>{{pg1, pg2, pic}, {}, {{pg1, pic}, {pg2, pic}}, {pic}});
    //coordinate
    auto c = std::shared_ptr<coordinator<Time, boost::any, priority_queue_vector>>( new coordinator<Time, boost::any, priority_queue_vector>(cm));
    Time t = c->init(Time{0});
    BOOST_CHECK_EQUAL( t, Time{1} ); //first advance

    //at time 1
    auto reply = c->collectOutputs(t);
    c->advanceSimulation( Time{1});
    t = c->next();
    BOOST_CHECK_EQUAL( t, Time{2});
    BOOST_REQUIRE_EQUAL( reply.size(), 0);

    //at time 2
    reply = c->collectOutputs(t);
    c->advanceSimulation( Time{2});
    t = c->next();
    BOOST_CHECK_EQUAL( t, Time{2}); //at 2,  0 is sent
    BOOST_REQUIRE_EQUAL( reply.size(), 0);

    //at again time 2 output is collected
    reply = c->collectOutputs(t);
    c->advanceSimulation( Time{2});
    t = c->next();
    BOOST_CHECK_EQUAL( t, Time{3}); //next g1 tick
    BOOST_REQUIRE_EQUAL( reply.size(), 1);
    BOOST_CHECK_EQUAL( boost::any_cast<int>(reply[0]), 2);
}

BOOST_AUTO_TEST_CASE( something_with_confluence_test )
{
    //create a generator and a processor, with same time
    std::shared_ptr<atomic<Time, Message>> pg{ new generator<Time, Message>{Time{2}, 1} };
    std::shared_ptr<atomic<Time, Message>> pp{ new processor<Time, Message>{Time{2}} };
    std::shared_ptr<atomic<Time, Message>> pt{ new ConfluenceTestHelper<Time, Message>{Time{2}} };
    //couple them
    auto cm = std::shared_ptr<coupled<Time, Message>>(new coupled<Time, Message>{{pg, pp, pt}, {}, {{pg, pp}, {pg, pt}, {pp, pt}}, {pt}});

    //coordinate
    auto c = std::shared_ptr<coordinator<Time, Message, priority_queue_vector>>( new coordinator<Time, boost::any, priority_queue_vector>(cm));
    Time t = c->init(Time{0});
    BOOST_CHECK_EQUAL( t, Time{2});
    //at time 2

    auto reply = c->collectOutputs(t);
    c->advanceSimulation( Time{2});
    BOOST_REQUIRE_EQUAL( reply.size(), 0);

    //at time 4
    c->advanceSimulation( Time{4}); //check if confluence is called.

}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()


//until finding out how to properly do these tests with template parameters, duplicating with different parameters

BOOST_AUTO_TEST_SUITE( p_coordinator_using_nullqueue_test_suite )
BOOST_AUTO_TEST_SUITE( p_coordinated_generator_test_suite )
//generators
BOOST_AUTO_TEST_CASE( p_coordinated_generator_produces_right_output_test ){
    //create a generator into a coupled model.
    //connect only its output
    //check the right output is generated when advancing.

    std::shared_ptr<atomic<Time, Message>> pa{ new generator<Time, Message>{Time{1}, 2}};
    auto cm = std::shared_ptr<coupled<Time, Message>>(new coupled<Time, Message>{{pa}, {}, {}, {pa}});
    auto c = std::shared_ptr<coordinator<Time, boost::any, nullqueue>>( new coordinator<Time, boost::any, nullqueue>(cm));

    Time t = c->init(Time{0});
    BOOST_CHECK_EQUAL( t, Time{1} ); //first advance
    auto reply = c->collectOutputs(t);
    c->advanceSimulation( Time{1});
    t = c->next();
    BOOST_CHECK_EQUAL( t, Time(2)); //after input half the tick passed
    BOOST_REQUIRE_EQUAL( reply.size(), 1);
    BOOST_CHECK_EQUAL( boost::any_cast<int>(reply[0]), 2);
}

BOOST_AUTO_TEST_CASE( p_coordinated__multiple_generators_produces_right_output_test )
{
    //create 3 generator into 3 coupled models in cascade.
    //connect only its output
    //check the time advanced until input was consumed

    //first model
    std::shared_ptr<atomic<Time, Message>> pa1{ new generator<Time, Message>{Time{1}, 1}};
    auto cm1 = std::shared_ptr<coupled<Time, Message>>(new coupled<Time, Message>{{pa1}, {}, {}, {pa1}});
    //second model
    std::shared_ptr<atomic<Time, Message>> pa2{ new generator<Time, Message>{Time{2}, 2}};
    auto cm2 = std::shared_ptr<coupled<Time, Message>>(new coupled<Time, Message>{{pa2, cm1}, {}, {}, {pa2, cm1}});
    //third model
    std::shared_ptr<atomic<Time, Message>> pa3{ new generator<Time, Message>{Time{3}, 3}};
    auto cm3 = std::shared_ptr<coupled<Time, Message>>(new coupled<Time, Message>{{pa3, cm2}, {}, {}, {pa3, cm2}});
    //coordination
    auto c = std::shared_ptr<coordinator<Time, boost::any, nullqueue>>( new coordinator<Time, boost::any, nullqueue>(cm3));

    Time t = c->init(Time{0});
    BOOST_CHECK_EQUAL( t, Time{1} ); //first advance
    //at time 1
    auto reply = c->collectOutputs(t);
    c->advanceSimulation( Time{1});
    t = c->next();
    BOOST_CHECK_EQUAL( t, Time{2}); //after input half the tick passed
    BOOST_REQUIRE_EQUAL( reply.size(), 1);
    BOOST_CHECK_EQUAL( boost::any_cast<int>(reply[0]), 1);
    //at time 2
    reply = c->collectOutputs(t);
    c->advanceSimulation( Time{2});
    t = c->next();
    BOOST_CHECK_EQUAL( t, Time{3}); //after input half the tick passed
    BOOST_REQUIRE_EQUAL( reply.size(), 2);
    BOOST_CHECK_EQUAL( std::count_if(reply.begin(), reply.end(), [](boost::any& m) { return boost::any_cast<int>(m) ==  1;}), 1);
    BOOST_CHECK_EQUAL( std::count_if(reply.begin(), reply.end(), [](boost::any& m) { return boost::any_cast<int>(m) ==  2;}), 1);
    //at time 3

    reply = c->collectOutputs(t);
    c->advanceSimulation( Time{3});
    t = c->next();
    BOOST_CHECK_EQUAL( t, Time{4}); //after input half the tick passed
    BOOST_REQUIRE_EQUAL( reply.size(), 2);
    BOOST_CHECK_EQUAL( std::count_if(reply.begin(), reply.end(), [](boost::any& m) { return boost::any_cast<int>(m) ==  1;}), 1);
    BOOST_CHECK_EQUAL( std::count_if(reply.begin(), reply.end(), [](boost::any& m) { return boost::any_cast<int>(m) ==  3;}), 1);
    //at time 4

    reply = c->collectOutputs(t);
    c->advanceSimulation( Time{4});
    t = c->next();
    BOOST_CHECK_EQUAL( t, Time{5}); //after input half the tick passed
    BOOST_REQUIRE_EQUAL( reply.size(), 2);
    BOOST_CHECK_EQUAL( std::count_if(reply.begin(), reply.end(), [](boost::any& m) { return boost::any_cast<int>(m) ==  1;}), 1);
    BOOST_CHECK_EQUAL( std::count_if(reply.begin(), reply.end(), [](boost::any& m) { return boost::any_cast<int>(m) ==  2;}), 1);
    //at time 5

    reply = c->collectOutputs(t);
    c->advanceSimulation( Time{5});
    t = c->next();
    BOOST_CHECK_EQUAL( t, Time{6}); //after input half the tick passed
    BOOST_REQUIRE_EQUAL( reply.size(), 1);
    BOOST_CHECK_EQUAL( std::count_if(reply.begin(), reply.end(), [](boost::any& m) { return boost::any_cast<int>(m) ==  1;}), 1);
    //at time 6

    reply = c->collectOutputs(t);
    c->advanceSimulation( Time{6});
    t = c->next();
    BOOST_CHECK_EQUAL( t, Time{7}); //after input half the tick passed
    BOOST_REQUIRE_EQUAL( reply.size(), 3);
    BOOST_CHECK_EQUAL( std::count_if(reply.begin(), reply.end(), [](boost::any& m) { return boost::any_cast<int>(m) ==  1;}), 1);
    BOOST_CHECK_EQUAL( std::count_if(reply.begin(), reply.end(), [](boost::any& m) { return boost::any_cast<int>(m) ==  2;}), 1);
    BOOST_CHECK_EQUAL( std::count_if(reply.begin(), reply.end(), [](boost::any& m) { return boost::any_cast<int>(m) ==  3;}), 1);


}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( pcoordinated_generators_and_infinite_counters_test_suite )
//generators and infinite_counters
BOOST_AUTO_TEST_CASE( generator_send_to_infinite_counter_manual_reset_test )
{
    //create a generator and a infinite_counter
    std::shared_ptr<atomic<Time, Message>> pg{ new generator<Time, Message>{Time{2}, 1} };
    std::shared_ptr<atomic<Time, Message>> pic{ new infinite_counter<Time, Message>{} };

    //input
    std::shared_ptr<std::istringstream> piss{ new std::istringstream{} };
    piss->str(" 3 0 ");
    auto pf = make_atomic_ptr<istream<Time, Message, int, int>, std::shared_ptr<std::istringstream>, Time>(piss, Time(0));

    //couple them
    auto cm = std::shared_ptr<coupled<Time, Message>>(new coupled<Time, Message>{{pg, pic, pf}, {}, {{pg, pic}, {pf, pic}}, {pic}});
    //coordinate
    auto c = std::shared_ptr<coordinator<Time, boost::any, nullqueue>>( new coordinator<Time, boost::any, nullqueue>(cm));
    Time t = c->init(Time{0});
    BOOST_CHECK_EQUAL( t, Time{2} ); //first advance

    //at time 2
    auto reply = c->collectOutputs(t);
    c->advanceSimulation( Time{2});
    t = c->next();
    BOOST_CHECK_EQUAL( t, Time{3}); //input from pf
    BOOST_REQUIRE_EQUAL( reply.size(), 0);

    //at time 3
    reply = c->collectOutputs(t);
    c->advanceSimulation( Time{3});
    t = c->next();
    BOOST_CHECK_EQUAL( t, Time{3}); //asking for count

    //again at 3
    reply = c->collectOutputs(t);
    c->advanceSimulation( Time{3});
    t = c->next();
    BOOST_CHECK_EQUAL( t, Time{4}); //next generator tick
    BOOST_REQUIRE_EQUAL( reply.size(), 1);
    BOOST_CHECK_EQUAL( boost::any_cast<int>(reply[0]), 1);
}
BOOST_AUTO_TEST_CASE( generators_send_to_infinite_counter_test )
{
    //create a generator and a infinite_counter
    std::shared_ptr<atomic<Time, Message>> pg1{ new generator<Time, Message>{Time{1}, 1} };
    std::shared_ptr<atomic<Time, Message>> pg2{ new generator<Time, Message>{Time{2}, 0} };
    std::shared_ptr<atomic<Time, Message>> pic{ new infinite_counter<Time, Message>{} };

    //couple them
    auto cm = std::shared_ptr<coupled<Time, Message>>(new coupled<Time, Message>{{pg1, pg2, pic}, {}, {{pg1, pic}, {pg2, pic}}, {pic}});
    //coordinate
    auto c = std::shared_ptr<coordinator<Time, boost::any, nullqueue>>( new coordinator<Time, boost::any, nullqueue>(cm));
    Time t = c->init(Time{0});
    BOOST_CHECK_EQUAL( t, Time{1} ); //first advance

    //at time 1
    auto reply = c->collectOutputs(t);
    c->advanceSimulation( Time{1});
    t = c->next();
    BOOST_CHECK_EQUAL( t, Time{2});
    BOOST_REQUIRE_EQUAL( reply.size(), 0);

    //at time 2
    reply = c->collectOutputs(t);
    c->advanceSimulation( Time{2});
    t = c->next();
    BOOST_CHECK_EQUAL( t, Time{2}); //at 2,  0 is sent
    BOOST_REQUIRE_EQUAL( reply.size(), 0);

    //at again time 2 output is collected
    reply = c->collectOutputs(t);
    c->advanceSimulation( Time{2});
    t = c->next();
    BOOST_CHECK_EQUAL( t, Time{3}); //next g1 tick
    BOOST_REQUIRE_EQUAL( reply.size(), 1);
    BOOST_CHECK_EQUAL( boost::any_cast<int>(reply[0]), 2);
}

BOOST_AUTO_TEST_CASE( something_with_confluence_test )
{
    //create a generator and a processor, with same time
    std::shared_ptr<atomic<Time, Message>> pg{ new generator<Time, Message>{Time{2}, 1} };
    std::shared_ptr<atomic<Time, Message>> pp{ new processor<Time, Message>{Time{2}} };
    std::shared_ptr<atomic<Time, Message>> pt{ new ConfluenceTestHelper<Time, Message>{Time{2}} };
    //couple them
    auto cm = std::shared_ptr<coupled<Time, Message>>(new coupled<Time, Message>{{pg, pp, pt}, {}, {{pg, pp}, {pg, pt}, {pp, pt}}, {pt}});

    //coordinate
    auto c = std::shared_ptr<coordinator<Time, boost::any, nullqueue>>( new coordinator<Time, boost::any, nullqueue>(cm));
    Time t = c->init(Time{0});
    BOOST_CHECK_EQUAL( t, Time{2});
    //at time 2

    auto reply = c->collectOutputs(t);
    c->advanceSimulation( Time{2});
    BOOST_REQUIRE_EQUAL( reply.size(), 0);

    //at time 4
    c->advanceSimulation( Time{4}); //check the confluence function is called

}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()


