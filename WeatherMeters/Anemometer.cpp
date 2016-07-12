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
/* Implementation of class to interface for anemometer sensor with Sparkfun Weather Meters: 
    http://www.sparkfun.com/products/8942
*/
#include <mbed.h>
#include "Anemometer.h"


#define REV_PER_SECOND_TO_KMPH  2.4f


CAnemometer::CAnemometer(PinName AnemometerPin, unsigned int IgnoreTimeForDebounceInMicroSeconds)
    : CDebouncedFallingEdgeCounter(AnemometerPin, IgnoreTimeForDebounceInMicroSeconds)
{
    m_CurrentWindSpeed = 0.0f;
    ResetMaximumWindSpeed();
    m_OneSecondTicker.attach_us<CAnemometer>(this, &CAnemometer::OneSecondTickerISR, 1000000);
}


void CAnemometer::ResetMaximumWindSpeed()
{
    m_MaximumWindSpeed = m_CurrentWindSpeed;
}


void CAnemometer::OneSecondTickerISR()
{
    unsigned int GetCurrentCount = GetCountAndZero();
    
    m_CurrentWindSpeed = (float)GetCurrentCount * REV_PER_SECOND_TO_KMPH;
    if (m_CurrentWindSpeed > m_MaximumWindSpeed)
    {
        m_MaximumWindSpeed = m_CurrentWindSpeed;
    }
}

float CAnemometer::GetCurrentWindSpeed()
{
    return m_CurrentWindSpeed;
}


float CAnemometer::GetMaximumWindSpeed()
{
    return m_MaximumWindSpeed;
}
