#ifndef HUGGIARDUINO_H
#define HUGGIARDUINO_H


#define HUGGI_PRESSURE_SENSOR_DEBUG
#include "HuggiPressureSensor.h"

#define nl '\n' // newline


// --------------------------

#define BT_BAUDRATE    38400 // serial baudrate
#define TS_BAUDRATE    14400 // altserial baudrate


#define ID      "2-"
#define myName  "Lucy Linder"

// --------------------------


#define Syn         "H"
#define Ack         "OK"


#define SYN_RL_TIMEOUT           400 // readline timeout
#define ACK_RL_TIMEOUT           200 // readline timeout

// --------------------------

#define DATA_MAX_SIZE       50      // max size of the data (name)
#define ID_SIZE             10

#define LENGTH_SIZE         1
#define CHK_SIZE            4 // checksum is 4 hex characters

#define DATA_BUFF_SIZE      LENGTH_SIZE+DATA_MAX_SIZE+CHK_SIZE+1 // 1:length, data, 4:checksum, 1:newline

#define DATA_FORMAT         "%c%s%04X" // 1:length, data, 4: hex checksum

// --------------------------

#define INPUT_NBR   2

// --------------------------


#define RED     0xFF0000
#define GREEN   0x00FF00
#define BLUE    0x0000FF 
#define ORANGE  0xCC6600 
#define YELLOW  0xFFFF00

#define PURPLE  0x400561
#define PINK    0x961B7B

// -------------------------



// -------------------------

#define EEPROM_ID_ADDR      0
#define EEPROM_DATA_ADDR    100 
#define EEPROM_MAX_ADDRESS  300
#endif