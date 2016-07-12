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
/* Declaration of class to interface with wind vane sensor Sparkfun Weather Meters: 
    http://www.sparkfun.com/products/8942
*/
#include <mbed.h>
#include "WindVane.h"


#define REV_PER_SECOND_TO_KMPH  2.4f
#define PARALLEL_R(R1, R2) (((R1) * (R2)) / ((R1) + (R2)))
#define ARRAY_SIZE(X) (sizeof(X)/sizeof(X[0]))

struct SDescription
{
    const char* DirectionString;
    float       DirectionAngle;
    float       SensorResistance;
};


static SDescription g_WindVaneDescriptionTable[16] = {
    { "North",            0.0f,  33.0f * 1000.0f },
    { "North North East", 22.5f, PARALLEL_R(33.0f * 1000.0f, 8.2f * 1000.0f) },
    { "North East",       45.0f, 8.2f * 1000.0f },
    { "East North East",  67.5f, PARALLEL_R(8.2f * 1000.0f, 1000.0f) },
    { "East",             90.0f, 1000.0f },
    { "East South East",  112.5f, PARALLEL_R(1000.0f, 2.2f * 1000.0f) },
    { "South East",       135.0f, 2.2f * 1000.0f },
    { "South South East", 157.5f, PARALLEL_R(2.2f * 1000.0f, 3.9f * 1000.0f) },
    { "South",            180.0f, 3.9f * 1000.0f },
    { "South South West", 202.5f, PARALLEL_R(3.9f * 1000.0f, 16.0f * 1000.0f) },
    { "South West",       225.0f, 16.0f * 1000.0f },
    { "West South West",  247.5f, PARALLEL_R(16.0f * 1000.0f, 120.0f * 1000.0f) },
    { "West",             270.0f, 120.0f * 1000.0f },
    { "West North West",  292.5f, PARALLEL_R(120.0f * 1000.0f, 64.9f * 1000.0f) },
    { "North West",       315.0f, 64.9f * 1000.0f },
    { "North North West", 337.5f, PARALLEL_R(64.9f * 1000.0f, 33.0f * 1000.0f) }
};


CWindVane::CWindVane(PinName WindVanePin, float WindVaneSeriesResistance)
    : m_WindVaneAnalogIn(WindVanePin)
{
    for (size_t i = 0 ; i < ARRAY_SIZE(g_WindVaneDescriptionTable) ; i++)
    {
        SDescription* pDescription = &g_WindVaneDescriptionTable[i];

        m_WindVaneVoltageTable[i] = pDescription->SensorResistance /
                                    (pDescription->SensorResistance + WindVaneSeriesResistance);
    }
}

const char* CWindVane::GetWindDirectionAsString()
{
    return g_WindVaneDescriptionTable[DetermineWindDirectionIndex()].DirectionString;
}

float CWindVane::GetWindDirectionAsAngle()
{
    return g_WindVaneDescriptionTable[DetermineWindDirectionIndex()].DirectionAngle;
}

size_t CWindVane::DetermineWindDirectionIndex()
{
    float WindVaneSensorVoltage = m_WindVaneAnalogIn.read();

    // Default to the first entry being closest.
    size_t ClosestIndex = 0;
    float  ClosestDelta = fabs(m_WindVaneVoltageTable[0] - WindVaneSensorVoltage);
    
    // Walk the rest of the entries to see if there is another which is closer.
    for (size_t i = 1 ; i < ARRAY_SIZE(m_WindVaneVoltageTable) ; i++)
    {
        float Delta = fabs(m_WindVaneVoltageTable[i] - WindVaneSensorVoltage);
        if (Delta < ClosestDelta)
        {
            ClosestDelta = Delta;
            ClosestIndex = i;
        }
    }
    
    return ClosestIndex;
}
