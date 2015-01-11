/**
 * @file    HuggiPressureSensor.h
 * @author  Lucy Linder (lucy.derlin@gmail.com)
 * @date    November, 2014
 * @brief   Handle a pressure sensor.
 * 
 * This library supports sensor with any number of outputs.
 * 
 * Define the variable HUGGI_PRESSURE_SENSOR_DEBUG to print
 * verbose information about the value of the sensors.
 */

#ifndef HUGGI_PRESSURE_SENSOR
#define HUGGI_PRESSURE_SENSOR

#include <Arduino.h>
#include <Serial.h>
#include <Streaming.h>

// #define HUGGI_PRESSURE_SENSOR_DEBUG

#define MAX_SAMPLES          4     //!< Number of samples for calibation
#define DEFAULT_SENSITIVITY  0.75  //!< Sensitivity in percent of variation

/**
 * Structure used to hold the pin number and the calibrated value of each input 
 */
typedef struct {
    int pin;        //!< The pin number
    int refValue;   //!< The value read from the pin during calibration
} SInput;

/**
 * Helper class to deal with the custom pressure sensor.
 */
class HuggiPressureSensor 
{
    public:
        /**
         * @param pins The pins numbers.
         * @param nbrOfInputs Number of inputs, or zones on the sensor. Should match pins.length.
         */
        HuggiPressureSensor(int * pins, int nbrOfInputs);
        /**
         * @return true if the sensor is pressed, false otherwise.
         */
        bool isPressed();
        /**
         * Change the sensitivity of the sensor. 
         * @param sensitivity The sensitivity, in percent.
         */
        void setSensitivity(double sensitivity);
        /**
         * Calibrate the sensor. 
         * #MAX_SAMPLES samples are read from each of the pin and the average .
         * value will be set as a reference.
         */
        void calibrate();

    private:
        int nbrOfInputs;    //!< Number of zones/inputs of this sensor.
        double sensitivity; //!< Current sensitivity, in percent.
        SInput * inputs;    //!< Structure holding the pin number and reference value of each input.
};

#endif