/* Copyright 2012 Adam Green (http://mbed.org/users/AdamGreen/)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
/* Header file for wrapper class of InterruptIn used for counting falling edge
   events with specified hold off time for the purpose of debouncing.
*/
#ifndef _DEBOUNCED_FALLING_EDGE_COUNT_H_
#define _DEBOUNCED_FALLING_EDGE_COUNT_H_

namespace AFP
{

class CDebouncedFallingEdgeCounter
{
protected:
    InterruptIn     m_FallingEdgeInterruptIn;
    // UNDONE: Run for over 30 minutes and make sure that this rolls over as expected.
    Timer           m_Timer;
    unsigned int    m_IgnoreTimeForDebounce;
    unsigned int    m_LastFallingEdgeTime;
    unsigned int    m_FallingEdgeCount;

public:
    CDebouncedFallingEdgeCounter(PinName InterruptInPin, unsigned int IgnoreTimeForDebounceInMicroSeconds);
    
    unsigned int GetCountAndZero();
    void         ZeroCount();
    unsigned int GetCount();

protected:
    void          InitLastFallingEdgeTime();
    void          InitLastFallingEdgeTime(unsigned int CurrentTime);
    void          FallingEdgeISR();
    void          RefreshTimers();
    unsigned int  TimeSinceLastFallingEdge(unsigned int CurrentTime);
};

} // namespace AFP
using namespace AFP;

#endif // _DEBOUNCED_FALLING_EDGE_COUNT_H_
