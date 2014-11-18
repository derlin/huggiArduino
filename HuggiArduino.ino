// hugginess.ino

#include <Serial.h>
#include <Arduino.h>
#include <Streaming.h>
#include <AltSoftSerial.h>
#include <string.h>
#include "EEPROM.h"
#include "Adafruit_NeoPixel.h"

#include "HuggiArduino.h"
#include "HuggiBuffer.h"

#define GoFirst(otherId)   strcmp(ID, otherId) < 0

extern Adafruit_NeoPixel strip;

AltSoftSerial altSerial; //On Arduino Uno TX = pin 9 | RX = pin 8

// buffer for the raw other arduino's id
char myId[ID_BUFF_SIZE]       = {0};  

// raw incoming data
char bufferIn[DATA_BUFF_SIZE] = {0};

// outgoing data: contains length, data and checksum
char dataOut[DATA_BUFF_SIZE]  = {0};


// #define SENSITIVITY 
int inputs[] = {A0, A5}; 
HuggiPressureSensor sensor(inputs, INPUT_NBR);

HuggiBuffer huggiBuff;
Hug_t * currentHug;

// ---------------

void setup() 
{  
  Serial.begin(BT_BAUDRATE);
  altSerial.begin(TS_BAUDRATE);

  Serial << "Hugginess :)" << nl;
  // data default
  encodeData(dataOut, myName);
  encodeData(myId, ID);

  currentHug = huggiBuff.getAvail();

  // pressure sensor
  sensor.calibrate();

  // leds
  ledSetup();
}

// ------------

void loop() 
{

    if(currentHug == NULL)
    {
        Serial << "BUFF FULL !\n";
        ledBlink(RED, 4);
        delay(2000); 
    }
    else if(sensor.isPressed() && handshake(currentHug->id))
    {
        Serial << "HS OK WITH " << currentHug->id  << nl;

        ledSetColor(ORANGE);
        long start = millis();

        if(exchange(currentHug->data))
        {
            currentHug->duration = millis() - start;
            ledBlink(GREEN, 3);
            Serial << "DATA = " << currentHug->data << nl;
            Serial << " DURATION =~ " << currentHug->duration << nl;

            huggiBuff.commit();
            currentHug = huggiBuff.getAvail();
            delay(300);
        }

    }

    ledSetColor(0);

    // bluetooth
    if(Serial.available())
    {
        char c = Serial.read();
        Serial << "[got] " << c << nl;
        if(c == nl)
        {
            Serial << "bt " << huggiBuff.getSize() << "\n";
            // while(!huggiBuff.isEmpty())
            // {
            //     toString(Serial, *huggiBuff.getNext());
            // }
            // currentHug = huggiBuff.getAvail();
            sendHugs();
        }
    }

}

// --------------
bool exchange(char* data)
{
    long lastReceived = millis();

    bool dataReceived = false;

    int indexIn = 0;
    int indexOut = 0;


    Serial << "== Begin HUG..." << dataOut << nl;


    while(sensor.isPressed() && (millis() - lastReceived) < 600)
    {
        // send
        if(dataOut[indexOut] == 0)
        {
            // end of data, start again from the beginning
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

                    if((dataReceived = decodeData(bufferIn, data)))
                    {
                        Serial << " [1] rcvd " << bufferIn << nl;
                        ledSetColor(GREEN);
                    }
                }
            }
        }
    }

    Serial << "== End of HUG..." << nl;

    return dataReceived;
}


// -----------------

bool handshake(char * otherId)
{

    ledSetColor(YELLOW);
    bufferIn[0] = 0; // reset

    //Serial << "in handshake" << nl;
    altSerial << Syn << myId << nl;

    // the sending of "H<id>" should be very frequent for the handshake to work
    bool syn = altSerial.available() && altSerial.read() == 'H' && readLine(altSerial, bufferIn, ID_BUFF_SIZE, SYN_RL_TIMEOUT);

    if(!syn) return false;


    if(strcmp(myId, bufferIn) == 0)
    {
        Serial << "LOOPBACK DETECTED" << nl;
        ledBlink(ORANGE, 3);
        return false;
    }
    // check that id is not truncated
    bool ok = decodeData(bufferIn, otherId);
    if(ok)
    {
        Serial << "ID = " << otherId << " | my id = | " << myId << nl;
    }
    else
    {
        Serial << "ID NOT OK = " << bufferIn << nl;
        otherId[0] = 0; // reset
        ledSetColor(0);
        return false;
    }

    // ack
    bool ack, goFirst = GoFirst(otherId);
    Serial << "[HS] " << otherId << " first == " << goFirst << nl;


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

