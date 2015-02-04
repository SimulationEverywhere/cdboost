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


#include <iostream>
#include <chrono>
#include <algorithm>
#include <boost/simulation.hpp>
#include <boost/simulation/pdevs/basic_models/generator.hpp>
#include <boost/any.hpp>

using namespace boost::simulation;
using namespace boost::simulation::pdevs;
using namespace boost::simulation::pdevs::basic_models;
using namespace std;

using hclock=chrono::high_resolution_clock;

//This example shows how to process custom event lists with the pdevs istream model.

int main(){

    cout << "Creating an input stream to be processed by the istream atomic model" << endl;
    shared_ptr<istringstream> piss{ new istringstream{} };
    piss->str("1 hello \n 1 world \n 2 hello \n 2 world");

    cout << "Creating the pdevs istream model" << endl;
    // In this model the stream has integer Times and sequence of char Messages.
    // We need to convert those to double and boost::any, the process function is called
    // in each line to extract one time and one message at the time.

    auto pf = make_atomic_ptr<istream<double, boost::any, int, string>, shared_ptr<istringstream>, double>(piss, double(0),
                [](const string& s, double& t_next, boost::any& m_next)->void{ //parsing function
            //intermediary vars for casting
            int tmp_next;
            string tmp_next_out;
            stringstream ss;
            ss.str(s);
            ss >> tmp_next;
            t_next = static_cast<double>(tmp_next);
            ss >> tmp_next_out;
            m_next = static_cast<boost::any>(tmp_next_out);
            string thrash;
            ss >> thrash;
            if ( 0 != thrash.size()) throw exception();
        });

    cout << "Coupling the models and connecting to the coupled output" << endl;

    shared_ptr<coupled<double, boost::any>> player( new coupled<double, boost::any>{{pf}, {}, {}, {pf}});

    cout << "Preparing runner" << endl;
    double initial_time{0};
    runner<double, boost::any> r(player, initial_time, cout, [](ostream& os, boost::any m){ os << boost::any_cast<string>(m);});

    cout << "Starting simulation until all events are consumed" << endl;

    auto start = hclock::now(); //to measure simulation execution time

    r.runUntilPassivate();

    auto elapsed = chrono::duration_cast<chrono::duration<double, ratio<1>>>
                                                                                          (hclock::now() - start).count();

    cout << "Finished simulation" << endl;
    cout << "Simulation took:" << elapsed << "sec" << endl;
    return 0;
}
