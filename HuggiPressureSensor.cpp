#include "HuggiPressureSensor.h"
#define nl '\n'


HuggiPressureSensor::HuggiPressureSensor(int pins[], int nbrOfInputs)
{
    this->nbrOfInputs = nbrOfInputs;
    this->inputs      = new SInput[nbrOfInputs];

    for (int i = 0; i < nbrOfInputs; i++)
    {
        inputs[i].pin = pins[i];
        pinMode(inputs[i].pin, INPUT_PULLUP);
    }

    this->sensitivity = DEFAULT_SENSITIVITY;
}

// -----------

#ifdef HUGGI_PRESSURE_SENSOR_DEBUG
    // be verbose
    bool HuggiPressureSensor::isPressed()
    {
        double value = 0;
        int count = 0;
        
        Serial << nl;

        for (int i = 0; i < nbrOfInputs; i++)
        {
            value = analogRead(inputs[i].pin);
            double delta = abs(inputs[i].refValue - value) / inputs[i].refValue; 

            if(delta > sensitivity)
            {
                Serial << "[" << i << "] ## " << value << " | " << delta << nl;
                count++;
            }
            else
            {
                Serial << "[" << i << "]    " << value << " | " << delta << nl;
            }
        }

        return count == nbrOfInputs;
    }
#else
    bool HuggiPressureSensor::isPressed()
    {
        for (int i = 0; i < nbrOfInputs; i++)
        {
            double val = analogRead(inputs[i].pin);
            #ifdef HUGGI_PRESSURE_SENSOR_DEBUG
            Serial << " [" << i << "] " << val << nl;
            #endif
            double delta = abs(inputs[i].refValue - val) / inputs[i].refValue; 

            if( delta < sensitivity) return false;
        }
        return true;
    }
#endif

// --------

void HuggiPressureSensor::setSensitivity(double sensitivity)
{
    this->sensitivity = sensitivity;
}

// -------

void HuggiPressureSensor::calibrate()
{
    for (int i = 0; i < nbrOfInputs; i++)
    {
          int count = 0, val = 0;

          for(int j = 0; j < MAX_SAMPLES; j++){
              val = analogRead(inputs[i].pin);
              count += val;
              delay(300);
          }

          inputs[i].refValue = floor(count / MAX_SAMPLES);

          #ifdef HUGGI_PRESSURE_SENSOR_DEBUG
            Serial << " [" << i << "] ref value = " << val << nl;
          #endif          
    }
}