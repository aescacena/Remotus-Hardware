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
/* Header file for class to interface with anemometer sensor of Sparkfun Weather Meters: 
    http://www.sparkfun.com/products/8942
*/
#ifndef _ANEMOMETER_H_
#define _ANEMOMETER_H_

#include "DebouncedFallingEdgeCounter.h"


namespace AFP
{

class CAnemometer : protected CDebouncedFallingEdgeCounter
{
protected:
    Ticker          m_OneSecondTicker;
    float           m_CurrentWindSpeed;
    float           m_MaximumWindSpeed;

public:
    CAnemometer(PinName AnemometerPin, unsigned int IgnoreTimeForDebounceInMicroSeconds = 1000);
    
    float GetCurrentWindSpeed();
    float GetMaximumWindSpeed();
    void  ResetMaximumWindSpeed();

protected:
    void OneSecondTickerISR();
};

} // namespace AFP
using namespace AFP;

#endif // _ANEMOMETER_H_
