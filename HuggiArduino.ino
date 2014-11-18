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
char myId[ID_SIZE+1]       = {0};  

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

  

  prepareDataOut(ID, myName);
  currentHug = huggiBuff.getAvail();

  // pressure sensor
  sensor.calibrate();

  // leds
  ledSetup();
}


void prepareDataOut(char * myId, char * myData)
{
    char temp[ID_SIZE+DATA_BUFF_SIZE];
    sprintf(temp, "%s%s", myId, myData);
    encodeData(dataOut, temp); 
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
    else if(sensor.isPressed())
    {
        ledSetColor(ORANGE);

        if(exchange(currentHug))
        {
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
            //sendHugs();
        }
    }

}

#define DATA_NOK 'H'
#define DATA_OK  'O'

// --------------

bool exchange(Hug_t * hug)
{
    int start = -1;

    int lastReceived = millis();
    bool dataReceived = false;
    bool otherHasReceivedData = false;

    int indexIn = -1;
    int indexOut = 0;


    Serial << "== Begin HUG..." << nl;


    while(sensor.isPressed())
    {
        int ms = millis() - lastReceived;
        if(ms > 500)
        {
            Serial << "TIMEOUT " << ms << nl;
            break;
        }

        // ------------------------------ send
        if(dataOut[indexOut] == 0)
        {
            // end of data, start again from the beginning
            altSerial << nl;
            indexOut = 0;
        }
        else
        {
            if(indexOut == 0) // first char: add a (n)ack
                altSerial << (dataReceived ? DATA_OK : DATA_NOK);

            altSerial << dataOut[indexOut++];
        }


        // ----------------------------- receive

        if(altSerial.available() )
        {

            if(start < 0) // first char received ever, start measuring
                start = millis();

            // get the next char
            char c = altSerial.read();
            lastReceived = millis();


            if(indexIn < 0) // first char: get the ack
            {
                otherHasReceivedData = (c == DATA_OK);
                indexIn = 0;
            }
            else if(dataReceived && c == nl) // data already ok
            {
                // just note that we got and end
                // so we capture the ack on the next char
                indexIn = -1;
            }
            else // data not received
            {
                bufferIn[indexIn++] = c;
                
                if(c == nl) // end of data -> process it
                {
                    bufferIn[indexIn] = 0; 
                    indexIn = -1;

                    byte dataLen = decodeData(bufferIn, hug->data);
                    if(dataLen > 0) // decode ok
                    {                        
                        // get the id
                        memcpy(hug->id, hug->data, ID_SIZE);
                        hug->id[ID_SIZE] = 0;

                        if(strcmp(hug->id, ID) == 0) // detect loopbacks
                        {
                            Serial << "LOOP BACK detected !\n";
                            ledBlink(RED, 3);
                            return false; 
                        }

                        // get the data, i.e. remove the id from the data
                        memmove(hug->data, hug->data + ID_SIZE, dataLen - ID_SIZE + 1);

                        // debug
                        Serial << "  rcvd " << hug->data << " from " << hug->id << nl;

                        // feedback
                        ledSetColor(GREEN);
                        dataReceived = true;
                    }
                }
            }

        } // end reveive

    } // end while


    Serial << "== End of HUG... other ok ? " << (otherHasReceivedData ? "TRUE" : "FALSE") << nl;

    if(dataReceived && otherHasReceivedData) // only if ok for both ? TODO
    {
        hug->duration = millis() - start;
        return true;
    }

    return false;
} // end exchange
