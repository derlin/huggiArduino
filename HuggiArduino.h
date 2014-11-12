#ifndef HUGGIARDUINO_H
#define HUGGIARDUINO_H


#define HUGGI_PRESSURE_SENSOR_DEBUG
#include "HuggiPressureSensor.h"

#define nl '\n' // newline


// --------------------------

#define BT_BAUDRATE    9600 // serial baudrate
#define TS_BAUDRATE    14400 // altserial baudrate


#define ID      "2-"
#define myName  "Lucy Linder"

// --------------------------


#define Syn         "H"
#define Ack         "OK"


#define SYN_RL_TIMEOUT           400 // readline timeout
#define ACK_RL_TIMEOUT           200 // readline timeout

// --------------------------

#define DATA_MAX_SIZE       70      // max size of the data (name)
#define ID_MAX_SIZE         5

#define LENGTH_SIZE         1
#define CHK_SIZE            4 // checksum is 4 hex characters

#define ID_BUFF_SIZE        LENGTH_SIZE+ID_MAX_SIZE+CHK_SIZE+1   // 1:length, id,  end char
#define DATA_BUFF_SIZE      LENGTH_SIZE+DATA_MAX_SIZE+CHK_SIZE+1 // 1:length, data, 4:checksum, 1:newline

#define DATA_FORMAT         "%c%s%04X" // 1:length, data, 4: hex checksum

// --------------------------

#endif