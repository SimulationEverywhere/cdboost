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


#ifndef BOOST_SIMULATION_PDEVS_event_stream_H
#define BOOST_SIMULATION_PDEVS_event_stream_H
#include <istream>
#include <sstream>
#include <boost/simulation/pdevs/atomic.hpp>

namespace boost {
namespace simulation {
namespace pdevs {
namespace basic_models {
/**
 * @brief event_stream PDEVS Model.
 *
 * event_stream PDEVS Model plays a history of events received by an input stream.
 * The list of events allows to be used as a connector to external tools.
 * The input format is "time output and a custom parser can be defined."
 *
*/
template<class TIME, class MSG, class T=int, class M=int> //T and M are the type expected to be read from the event_stream
class event_stream : public atomic<TIME, MSG>
{
    std::shared_ptr<std::istream> _ps; //the stream
    TIME _last;
    TIME _next;
    std::vector<MSG> _output;
    TIME _prefetched_time;
    MSG _prefetched_message;
    void (*_process)(const std::string&, TIME&, MSG&); //Parser process reads the string and sets the time,msg
    const TIME infinity=1000; // Defining local infinity for testing purposes


    //helper function
    void fetchUntilTimeAdvances() {
        //making use of the prefetched values
        _next = _prefetched_time;
        _output = {_prefetched_message};
        //fetching next messages
        std::string line;
        do
            std::getline(*_ps, line);
        while(!_ps->eof() && line.empty());
        if (_ps->eof() && line.empty()){
            //if there is no more messages, set infinity as next event time
            _prefetched_time = infinity;//atomic<TIME, MSG>::infinity;
        } else { //else cache the las message fetched
            //intermediary vars for casting
            TIME t_next;
            MSG m_next;
            _process(line, t_next, m_next);
            //advance until different time is fetched
            while( _next == t_next){
                _output.push_back(m_next);
                line.clear();
                std::getline(*_ps, line);
                if (_ps->eof() && line.empty()){
                    _prefetched_time = infinity;//atomic<TIME, MSG>::infinity;
                    return;
                } else {
                    _process(line, t_next, m_next);
                }
            }
            //cache the last message fetched
            if (t_next < _next) throw std::exception();
            _prefetched_time = t_next;
            _prefetched_message = m_next;
        }
    }

public:
    /**
     * @brief event_stream constructor sets the stream to be read and the initial time
     * @param pis is a pointer to the input stream to be read
     * @param init is the time the simulation of the model starts, the input MUST have absolute times greater than init time.
     */
    explicit event_stream(std::shared_ptr<std::istream> pis, TIME init) noexcept :
        event_stream(pis, init,
            [](const std::string& s, TIME& t_next, MSG& m_next){
                            T tmp_next;
                            M tmp_next_out;
                            std::stringstream ss;
                            ss.str(s);
                            ss >> tmp_next;
                            t_next = static_cast<TIME>(tmp_next);
                            ss >> tmp_next_out;
                            m_next = static_cast<MSG>(tmp_next_out);
                            std::string thrash;
                            ss >> thrash;
                            if ( 0 != thrash.size()) throw std::exception();
                        }
                    )
    {}
    /**
     * @brief event_stream constructor sets the stream to be read and the initial time and a custom parser
     * @param pis is a pointer to the input stream to be read
     * @param init is the time the simulation of the model starts, the input MUST have absolute times greater than init time.
     * @param process the process to parse each line of input and extract time and messages
     */
    explicit event_stream(std::shared_ptr<std::istream> pis, TIME init, decltype(_process) process)  noexcept : _ps{pis}, _last{init}, _process(process) {
        std::string line;
        std::getline(*_ps, line); //needs at least one call to detect eof
        if (_ps->eof() && line.empty()){
            _next = infinity;//atomic<TIME, MSG>::infinity;
        } else {
            //intermediary vars for casting
            TIME t_next;
            MSG m_next;
            process(line, t_next, m_next);
            //the first iteration needs this to run
            _prefetched_time = t_next;
            _prefetched_message = m_next;
            fetchUntilTimeAdvances();
        }
    }

    /**
     * @brief internal function reads the stream and prepares next event.
     */
    void internal() noexcept {
        _last = _next;
        fetchUntilTimeAdvances();
     }
    /**
     * @brief advance function time until next fetched item or infinity if EOS.
     * @return TIME until next internal event.
     */
    TIME advance() const noexcept {
        return (_next==infinity?_next:_next-_last);//return (_next==atomic<TIME, MSG>::infinity?_next:_next-_last);

    }
    /**
     * @brief out function.
     * @return the event defined in the input.
     */
    std::vector<MSG> out() const noexcept { return _output; }
    /**
     * @brief invalid external function.
     */
    void external(const std::vector<MSG>& mb, const TIME& t) noexcept { assert(false && "Non external input is expected in this model"); }
    /**
     * @brief invalid confluence function.
     */
    void confluence(const std::vector<MSG>& mb, const TIME& t)  noexcept { assert(false && "Non external input is expected in this model"); }

};

}
}
}
}

#endif // BOOST_SIMULATION_PDEVS_event_stream_H
