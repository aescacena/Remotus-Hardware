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
/* Header file for class to interface with Sparkfun Weather Meters: 
    http://www.sparkfun.com/products/8942
*/
#ifndef _WEATHER_METERS_H_
#define _WEATHER_METERS_H_

#include "WindVane.h"
#include "RainGauge.h"
#include "Anemometer.h"


namespace AFP
{

/** A class to interface with the Sparkfun Weather Meters.
    http://www.sparkfun.com/products/8942
    
    This set of weather meters from Sparkfun includes anemometer (wind speed),
    wind vane, and rain gauge.  All of the sensors use reed switches so that
    the electronics can detect motion/location in the sensor hardware via 
    magnets placed in the sensors without actually exposing the electonic
    switches themselves to the elements.  A reed switch closes as the 
    anemometer spins and the rain gauge empties.  The wind vane includes 
    8 reed switches, each with a different valued resistor attached so that
    the voltage will vary depending on where the magnet in the vane is
    located.
 * Example:
 * @code
#include <mbed.h>
#include "WeatherMeters.h"

int main() 
{
    CWeatherMeters WeatherMeters(p9, p10, p15);
    for (;;)
    {
        CWeatherMeters::SMeasurements   Measurements;
        
        WeatherMeters.GetMeasurements(&Measurements);
        
        printf("Direction: %s\n", Measurements.WindDirectionString);
        printf("Depth: %f\n",Measurements.Rainfall);
        printf("Current speed: %f\n", Measurements.WindSpeed);
        printf("Maximum speed: %f\n", Measurements.MaximumWindSpeed);

        wait(1.0f);
    }
}
 * @endcode
 */
class CWeatherMeters
{
protected:
    CAnemometer     m_Anemometer;
    CRainGauge      m_RainGauge;
    CWindVane       m_WindVane;

public:
     /** A structure in which the current measurements registered by the weather
         meters are returned from the GetMeasurements() method.
     */
     struct SMeasurements
     {
        const char* WindDirectionString;    /**< String representing current wind direction.  Example: "North" */
        float       WindDirectionAngle;     /**< Wind direction in degrees from north. */
        float       WindSpeed;              /**< Current wind speed in km/hour. */
        float       MaximumWindSpeed;       /**< Maximum wind speed measured since last reset.  Measured in km/hour. */
        float       Rainfall;               /**< Amount of rainfall since last reset.  Measured in mm. */
     };
     
     /** Create a CWeatherMeters object, binds it to the specified input pins,
         and initializes the required interrupt handlers.
         
         @param AnemometerPin The mbed pin which is connected to the anemometer
                sensor.  Must be a pin on which the mbed supports InterruptIn.
         @param RainGauagePin The mbed pin which is connected to the rain gauge
                sensor.  Must be a pin on which the mbed supports InterruptIn.
         @param WindVanePin The mbed pin which is connected to the wind vane
                sensor.  Must be a pin on which the mbed supports AnalogIn.
         @param WindVaneSeriesResistance The value of the resistance which is
                in series with the wind vane sensor and along with the
                varying resistance of the wind vane sensor itself, acts as a
                voltage divider.
    */
    CWeatherMeters(PinName AnemometerPin,
                   PinName RainGaugePin,
                   PinName WindVanePin,
                   float   WindVaneSeriesResistance = 10000.0f);
    
    /** Gets current weather meter measurements.
    
        NOTE: Should call this method on a regular basis (every 15 minutes or 
        less) to keep internal timers from overflowing.
    
     @param pMeasurements points to the measurement structure to be filled in
            with the current measurement from the sensors.  The historical
            values in this structure such as rainfall and maximum wind
            speed can be reset by calls to ResetRainfall(), 
            ResetMaximumWindSpeed(), and Reset().
    */
    void GetMeasurements(SMeasurements* pMeasurements);
    
    /** Resets all historical measurements: rainfall and maximum wind speed. */
    void Reset();
    
    /** Resets the cumulative rainfall measurement. */
    void ResetRainfall();
    
    /** Resets the maximum wind speed measurement. */
    void ResetMaximumWindSpeed();
};

} // namespace AFP
using namespace AFP;

#endif // _WEATHER_METERS_H_
