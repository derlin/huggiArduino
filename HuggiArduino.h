#ifndef HUGGIARDUINO_H
#define HUGGIARDUINO_H


#define HUGGI_PRESSURE_SENSOR_DEBUG
#include "HuggiPressureSensor.h"

#define nl '\n' // newline


// --------------------------

#define BT_BAUDRATE    38400 //!< Bluetooth serial baudrate
#define TS_BAUDRATE    19200 //!< Altserial baudrate, used between HuggiShirts
// TODO: test 38400

// debug only
#define ID      "2-"
#define myName  "Lucy Linder"
// end debug only

#define DATA_DEFAULT    "Hugginess :)" //!< Data written to eeprom if not explicitly configured 

// --------------------------

#define DATA_MAX_SIZE       50  //!< Max size of the data sent through HuggiShirt (name)
#define ID_SIZE             10  //!< Size of an ID (phone number, ex: 0763131212)

#define LENGTH_SIZE         1   //!< The length of the data sent is coded with one byte
#define CHK_SIZE            4   //!< The checksum is four hexadecimal characters

#define DATA_BUFF_SIZE      LENGTH_SIZE+DATA_MAX_SIZE+CHK_SIZE+1 //!< Size of the buffer for incoming data

#define DATA_FORMAT         "%c%s%04X" //!< Format used to encode data. Length (1B), data, checksum (4B)

// --------------------------

#define PRESSURE_INPUT_NBR   2   //!< Number of zones in the pressure sensor. All of them must be pressed for a valid pression.

// --------------------------

#define RED     0xFF0000
#define GREEN   0x00FF00
#define BLUE    0x0000FF 
#define ORANGE  0xCC6600 
#define YELLOW  0xFFFF00

#define PURPLE  0x400561
#define PINK    0x961B7B

// -------------------------

#define EEPROM_ID_ADDR      0     //<! ID is written in eeprom at address 0.
#define EEPROM_DATA_ADDR    100   //<! data is written in eeprom at address 100.
#define EEPROM_MAX_ADDRESS  300   //<! Max eeprom address 




extern volatile bool triggered; //<! Boolean set to true after an interrupt from the pressure sensor

#endif