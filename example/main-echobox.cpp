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
#include <boost/simulation/pdevs/basic_models/processor.hpp>

using namespace boost::simulation;
using namespace boost::simulation::pdevs;
using namespace boost::simulation::pdevs::basic_models;
using namespace std;


using hclock=chrono::high_resolution_clock;

//This example is the simulation of a echo box doing 2 echos of the input.

int main(){
    cout << "Creating the atomic models for the 2 echos" << endl;
    auto echo1 = make_atomic_ptr<processor<double, boost::any>, double>(double{1}) ; //the second param in make_atomic_ptr template  is the expected type for PProcessor construction.
    auto echo2 = make_atomic_ptr<processor<double, boost::any>, double>(double{3}) ;

    cout << "Coupling the models into the echobox: input to echo1, echo1 to echo2, and both to the output" << endl;
    shared_ptr<coupled<double, boost::any>> echobox( new coupled<double, boost::any>{{echo1, echo2}, {echo1}, {{echo1, echo2}}, {echo1, echo2}});

    cout << "Creating the model to insert the input from stream" << endl;
    auto piss = make_shared<istringstream>();
    piss->str("1 1 \n 4 4 \n 5 5 \n 6 6 \n 8 8 \n 9 9 ");
    auto pf = make_atomic_ptr<istream<double, boost::any, int, int>, shared_ptr<istringstream>, double>(piss, double{0});

    cout << "Coupling the echobox to the input" << endl;
    shared_ptr<coupled<double, boost::any>> root( new coupled<double, boost::any>{{pf, echobox}, {}, {{pf, echobox}}, {echobox}});

    cout << "Preparing runner" << endl;
    double initial_time{0};
    runner<double, boost::any> r(root, initial_time, cout, [](ostream& os, boost::any m){ os << boost::any_cast<int>(m);});

    std::cout << "Starting simulation until passivate" << std::endl;

    auto start = hclock::now(); //to measure simulation execution time

    r.runUntilPassivate();

    auto elapsed = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>
                                                                                          (hclock::now() - start).count();

    cout << "Simulation took:" << elapsed << "sec" << endl;
    return 0;
}

