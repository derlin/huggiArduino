// hugginess.ino

#include <Serial.h>
#include <Arduino.h>
#include <Streaming.h>
#include <AltSoftSerial.h>

#include "EEPROM.h"
#include "Adafruit_NeoPixel.h"

#include "HuggiArduino.h"
#include "HuggiBuffer.h"

#define GoFirst(otherId)   strcmp(ID, otherId) < 0

extern Adafruit_NeoPixel strip;

AltSoftSerial altSerial; //On Arduino Uno TX = pin 9 | RX = pin 8

// raw incoming data
char bufferIn[DATA_BUFF_SIZE] = {0};

// outgoing data: contains length, data and checksum
char dataOut[DATA_BUFF_SIZE]  = {0};


// #define SENSITIVITY 
int inputs[] = {A0, A2}; 
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
  // prepareDataOut(ID, myName);
  currentHug = huggiBuff.getAvail();

  // pressure sensor
  sensor.calibrate();


  // leds
  ledSetup();
}

bool setDataOut()
{
    dataOut[0] = 0; // reset

    char * buff = bufferIn;
    byte len = readFromEEprom(EEPROM_ID_ADDR, buff, DATA_BUFF_SIZE);
    if(len == 0){
        Serial << "id not set" << nl;
        ledBlink(PURPLE, 3);
        return false;
    }

    buff += len; 
    len = readFromEEprom(EEPROM_DATA_ADDR, buff, DATA_BUFF_SIZE);
    if(len == 0){
        Serial << "name not set" << nl;
        ledBlink(PINK, 3);
        return false;
    }  

    encodeData(dataOut, bufferIn); 
}

// ------------

void loop() 
{
    if(currentHug == NULL)
    {
        Serial << "BUFF FULL!\n";
        ledBlink(RED, 4);
        delay(2000); 
    }
    else if(dataOut[0] == 0)
    {
        setDataOut();
        ledBlink(PINK, 3);
        delay(400);
        ; // do nothing, since id | name not configured
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
    delay(100);

    ledSetColor(0);

    // bluetooth
    if(Serial.available())
    {
        Serial << "== bt" << nl;
        bt();
        Serial << "!= bt" << nl;
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

                    byte dataLen = decodeData(bufferIn, hug->id);
                    if(dataLen > 0) // decode ok
                    {                        
                        // split id and data, shift by one to add an "end of string"
                        memmove(hug->data, hug->data -1, dataLen - ID_SIZE + 1);
                        hug->id[ID_SIZE] = 0;

                        if(strcmp(hug->id, ID) == 0) // detect loopbacks
                        {
                            Serial << "LOOP BACK detected !\n";
                            ledBlink(RED, 3);
                            return false; 
                        }


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


    Serial << "== End of HUG..." << nl;

    if(dataReceived && otherHasReceivedData) // only if ok for both ? TODO
    {
        hug->duration = millis() - start;
        return true;
    }

    return false;
} // end exchange
