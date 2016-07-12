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
/* Implementation for wrapper class of InterruptIn used for counting falling edge
   events with specified hold off time for the purpose of debouncing.
*/
#include <mbed.h>
#include "DebouncedFallingEdgeCounter.h"


CDebouncedFallingEdgeCounter::CDebouncedFallingEdgeCounter(PinName InterruptInPin, unsigned int IgnoreTimeForDebounceInMicroSeconds) 
    : m_FallingEdgeInterruptIn(InterruptInPin)
{
    m_Timer.start();
    m_IgnoreTimeForDebounce = IgnoreTimeForDebounceInMicroSeconds;
    InitLastFallingEdgeTime();

    ZeroCount();

    m_FallingEdgeInterruptIn.mode(PullDown);
    m_FallingEdgeInterruptIn.fall<CDebouncedFallingEdgeCounter>(this, &CDebouncedFallingEdgeCounter::FallingEdgeISR);
}


void CDebouncedFallingEdgeCounter::InitLastFallingEdgeTime()
{
    InitLastFallingEdgeTime((unsigned int)m_Timer.read_us());
}


void CDebouncedFallingEdgeCounter::InitLastFallingEdgeTime(unsigned int CurrentTime)
{
    m_LastFallingEdgeTime = CurrentTime - m_IgnoreTimeForDebounce;
}


void CDebouncedFallingEdgeCounter::FallingEdgeISR()
{
    // UNDONE: Can I just use whatever timer the m_Timer uses under the covers?
    unsigned int CurrentTime = (unsigned int)m_Timer.read_us();
    
    if (TimeSinceLastFallingEdge(CurrentTime) >= m_IgnoreTimeForDebounce)
    {
        m_FallingEdgeCount++;
        m_LastFallingEdgeTime = CurrentTime;
    }
}


unsigned int CDebouncedFallingEdgeCounter::TimeSinceLastFallingEdge(unsigned int CurrentTime)
{
    return CurrentTime - m_LastFallingEdgeTime;
}


unsigned int CDebouncedFallingEdgeCounter::GetCountAndZero()
{
    unsigned int CurrentCount;
    
    CurrentCount = GetCount();
    ZeroCount();
    return CurrentCount;
}


unsigned int CDebouncedFallingEdgeCounter::GetCount()
{
    RefreshTimers();
    
    return m_FallingEdgeCount;
}


void CDebouncedFallingEdgeCounter::ZeroCount()
{
    m_FallingEdgeCount = 0;
}


void CDebouncedFallingEdgeCounter::RefreshTimers()
{
    unsigned int CurrentTime = (unsigned int)m_Timer.read_us();
    
    if (TimeSinceLastFallingEdge(CurrentTime) > m_IgnoreTimeForDebounce)
    {
        // Protect from timer overflow.
        InitLastFallingEdgeTime(CurrentTime);
    }
}
