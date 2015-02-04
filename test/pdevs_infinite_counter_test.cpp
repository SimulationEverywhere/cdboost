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
#include <boost/simulation/pdevs/basic_models/infinite_counter.hpp>
#include <boost/any.hpp>
#include <math.h>

using namespace boost::simulation;
using namespace boost::simulation::pdevs;
using namespace boost::simulation::pdevs::basic_models;
using namespace std;

using Time=double;
using Message=boost::any;

BOOST_AUTO_TEST_SUITE( p_infinite_counter_suite )
BOOST_AUTO_TEST_CASE( p_infinite_counter_counts_one_test )
{
    //create an infinite counter
    //check it goes passive
    //input  numbers without zero goes passive
    //input  0 in the numbers goes active with ta=0 and outputs previous counts in next out, both for external and confluent.

    infinite_counter<Time, Message> ic;
    BOOST_CHECK(isinf(ic.advance()));
    ic.external(vector<Message>{1, 2, 3, 4, 5, 6}, Time{1});
    BOOST_CHECK(isinf(ic.advance()));
    ic.external(vector<Message>{7, 8, 9, 0}, Time{1});
    BOOST_CHECK_EQUAL(ic.advance(), Time(0));
    BOOST_CHECK_EQUAL(boost::any_cast<int>(ic.out()[0]), 9);
    ic.confluence(vector<boost::any>({0, 1, 2, 3}), Time{0});
    BOOST_CHECK_EQUAL(ic.advance(), Time(0));
    BOOST_CHECK_EQUAL(boost::any_cast<int>(ic.out()[0]), 3);
    ic.internal();
    BOOST_CHECK( isinf(ic.advance()) );
}
BOOST_AUTO_TEST_CASE( infinite_counter_counts_all_up_to_ten_test )
{
    //create an infinite counter
    //check it goes passive
    //input  numebers multiple times
    //check it goes passive after each input
    //input  0
    //check it goes active with ta=0 and outputs the total number, .

    infinite_counter<Time, Message> ic;
    BOOST_CHECK(isinf(ic.advance()));
    for (int i=1; i < 11; i++){
        for (int j=0; j<i; j++){
            ic.external(vector<boost::any>{1, 2, 3}, Time{1});
            BOOST_CHECK(isinf(ic.advance()));
        }
        ic.external(vector<boost::any>{0}, Time{1});
        BOOST_CHECK_EQUAL(Time(0), ic.advance());
        BOOST_CHECK_EQUAL(i*3, boost::any_cast<int>(ic.out()[0]));
        ic.internal();
        BOOST_CHECK(isinf(ic.advance()));
    }
}
BOOST_AUTO_TEST_SUITE_END()

