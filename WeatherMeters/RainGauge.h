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
/* Header file for class to interface with the rain gauge sensor of the Sparkfun Weather Meters: 
    http://www.sparkfun.com/products/8942
*/
#ifndef _RAIN_GAUGE_H_
#define _RAIN_GAUGE_H_

#include "DebouncedFallingEdgeCounter.h"


#define MILLIMETERS_OF_RAIN_PER_COUNT 0.2794f


namespace AFP
{

class CRainGauge : protected CDebouncedFallingEdgeCounter
{
public:
    CRainGauge(PinName RainGaugePin, unsigned int IgnoreTimeForDebounceInMicroSeconds = 1000)
        : CDebouncedFallingEdgeCounter(RainGaugePin, IgnoreTimeForDebounceInMicroSeconds)
    {
    }
    
    void  Reset()
    {
        ZeroCount();
    }
    
    float GetRainfall()
    {
        return (float)GetCount() * MILLIMETERS_OF_RAIN_PER_COUNT;
    }
};

} // namespace AFP
using namespace AFP;

#endif // _RAIN_GAUGE_H_
