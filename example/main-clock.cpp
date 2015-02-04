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
#include <boost/rational.hpp>

using namespace boost::simulation;
using namespace boost::simulation::pdevs;
using namespace boost::simulation::pdevs::basic_models;
using namespace std;


using hclock=chrono::high_resolution_clock;

//This example is the simulation of a clock with 3 needles (H,M,S)

int main(){
    cout << "Creating the atomic models for the 3 needles" << endl;

    auto second = make_atomic_ptr<generator<boost::rational<int>, string>, boost::rational<int>, string>(boost::rational<int>{1}, string("second"));
    auto minute = make_atomic_ptr<generator<boost::rational<int>, string>, boost::rational<int>, string>(boost::rational<int>{60}, string("minute"));
    auto hour = make_atomic_ptr<generator<boost::rational<int>, string>, boost::rational<int>, string>(boost::rational<int>{3600}, string("hour"));

    cout << "Coupling the models into a clock model, the 3 needles make output" << endl;

    shared_ptr<coupled<boost::rational<int>, string>> clock( new coupled<boost::rational<int>, string>{{second, minute, hour}, {}, {}, {second, minute, hour}});

    cout << "Preparing runner" << endl;
    boost::rational<int> initial_time{0};
    runner<boost::rational<int>, string> r(clock, initial_time, cout, [](ostream& os, string m){ os << m;});
    boost::rational<int> end_time{7200}; //2 hours

    std::cout << "Starting simulation until time:" << end_time << "seconds" << std::endl;

    auto start = hclock::now(); //to measure simulation execution time

    end_time = r.runUntil(end_time);
    
    auto elapsed = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>
                                                                                          (hclock::now() - start).count();
    
    cout << "Finished simulation with time: " << end_time << "sec" << endl;
    cout << "Simulation took:" << elapsed << "sec" << endl;
    return 0;
}

