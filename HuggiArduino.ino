// hugginess.ino

#include <Serial.h>
#include <Arduino.h>
#include <Streaming.h>
#include <AltSoftSerial.h>
#include <string.h>
#include "EEPROM.h"

#define HUGGI_PRESSURE_SENSOR_DEBUG
#include "HuggiPressureSensor.h"


#define nl '\n' // newline

#define GoFirst()   strcmp(ID, otherId) < 0

#define Baudrate    9600

#define Syn         "H"
#define Ack         "OK"
#define Fin         "BYE"

#define Nak         "OOPS"

#define Keep_Alive  "LALA"

#define DATA_MAX_SIZE       70      // max size of the data (name)

#define ID_BUFF_SIZE        10+1    // id + end char
#define DATA_BUFF_SIZE      1+DATA_MAX_SIZE+4+1 // 1:length, data, 4:checksum, 1:newline

#define SYN_RL_TIMEOUT           400 // readline timeout
#define ACK_RL_TIMEOUT           200 // readline timeout

AltSoftSerial altSerial; //On Arduino Uno TX = pin 9 | RX = pin 8

// buffer for the other arduino's id
char myId[ID_BUFF_SIZE]       = {0};  
char otherId[ID_BUFF_SIZE]    = {0};  
// raw incoming data + data without length and checksum
char bufferIn[DATA_BUFF_SIZE] = {0};
char dataIn[DATA_BUFF_SIZE]   = {0};
// outgoing data: contains length, data and checksum
char dataOut[DATA_BUFF_SIZE]  = {0};


#define INPUT_NBR   2
// #define SENSITIVITY 
int inputs[] = {A0, A5}; 
HuggiPressureSensor sensor(inputs, INPUT_NBR);


#define ID      "2-Uno"
#define myName  "Lucy Linder"

// ---------------

void setup() 
{  
  Serial.begin(Baudrate);
  altSerial.begin(14400);//Baudrate);

  Serial << "Hugginess :)" << nl;
  // data default
  encodeData(dataOut, myName);
  encodeData(myId, ID);

  sensor.calibrate();
}

// ------------

void loop() 
{

  if(sensor.isPressed() && handshake())
  {
    Serial << "HANDSHAKE OK WITH " << otherId << " !!" << nl;

    long start = millis();
    exchange();
    int duration = millis() - start;
    Serial << "DATA = " << dataIn << nl;
    Serial << " HUG DURATION =~ " << duration << nl;
    // delay(600);
  }

}

// --------------

bool exchange()
{
    long lastReceived = millis();

    bool dataReceived = false;

    int indexIn = 0;
    int indexOut = 0;


    Serial << "== Begin HUG..." << nl;


    while(sensor.isPressed() && (millis() - lastReceived) < 600)
    {
        // send
        if(dataOut[indexOut] == 0)
        {
            altSerial << nl;
            indexOut = 0;
        }
        else
        {
            altSerial << dataOut[indexOut++];
        }

        // receive
        if(altSerial.available() )
        {
            char c = altSerial.read();
            lastReceived = millis();

            if(!dataReceived)
            {
                bufferIn[indexIn++] = c;
                if(c == nl)
                {
                    bufferIn[indexIn] = 0;
                    indexIn = 0;

                    if(dataReceived = decodeData(bufferIn, dataIn))
                        Serial << "    [1] rcvd " << bufferIn << nl;
                }
            }
        }
    }

    Serial << "== End of HUG..." << nl;
}


// -----------------

bool handshake()
{
    bufferIn[0] = 0; // reset

    //Serial << "in handshake" << nl;
    altSerial << Syn << myId << nl;

    // the sending of "H<id>" should be very frequent for the handshake to work
    bool syn = altSerial.available() && altSerial.read() == 'H' && readLine(altSerial, bufferIn, ID_BUFF_SIZE, SYN_RL_TIMEOUT);

    if(!syn) return false;

    // check that id is not truncated
    bool ok = decodeData(bufferIn, otherId);
    if(ok)
    {
        Serial << "ID = " << otherId << nl;
    }
    else
    {
        Serial << "ID NOT OK = " << bufferIn << nl;
        return false;
    }

    // ack
    bool ack, goFirst = GoFirst();
    Serial << "[HS] detected " << otherId << " goFirst == " << goFirst << nl;


    if(goFirst)
    {
        altSerial << Ack;
        ack = readExpected(altSerial, Ack, ACK_RL_TIMEOUT);
    }
    else
    {
        ack = readExpected(altSerial, Ack, ACK_RL_TIMEOUT);
        if(ack) altSerial << Ack;
    }

    return ack;
}


// -----------------


