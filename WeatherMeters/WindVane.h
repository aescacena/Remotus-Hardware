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
/* Implementation of class to interface with wind vane sensor Sparkfun Weather Meters: 
    http://www.sparkfun.com/products/8942
*/
#ifndef _WIND_VANE_H_
#define _WIND_VANE_H_

namespace AFP
{

class CWindVane
{
protected:
    AnalogIn            m_WindVaneAnalogIn;
    float               m_WindVaneVoltageTable[16];

public:
    CWindVane(PinName WindVanePin, float WindVaneSeriesResistance = 10000.0f);
    
    const char* GetWindDirectionAsString();
    float       GetWindDirectionAsAngle();
    
protected:
    size_t      DetermineWindDirectionIndex();
};

} // namespace AFP
using namespace AFP;

#endif // _WIND_VANE_H_
