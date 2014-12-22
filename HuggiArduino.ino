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
HuggiPressureSensor sensor(inputs, PRESSURE_INPUT_NBR);

HuggiBuffer huggiBuff;
Hug_t * currentHug;

// ---------------

void setup() 
{  
  Serial.begin(BT_BAUDRATE);
  altSerial.begin(TS_BAUDRATE);

  Serial << DATA_DEFAULT << nl;


  // == DEBUG 
  /*
  currentHug = huggiBuff.getAvail();
  for(int i = 0; i < 2; i++)
  {
    sprintf(currentHug->id, "076444848%d", i+1);
    sprintf(currentHug->data, "%d%s%d", i, "hello",i);
    currentHug->duration = (i+1) * 1000;
    huggiBuff.commit();
    currentHug = huggiBuff.getAvail();

  }
  //*/
  // pressure sensor
  sensor.calibrate();


  // leds
  ledSetup();
}

// == DEBUG
void prepareDataOut_debug(char * id, char * myData)
{
    char temp[strlen(id)+strlen(myData)+1];
    sprintf(temp, "%s%s", id, myData);
    encodeData(dataOut, temp); 
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
        // write default data
        writeToEEprom(EEPROM_DATA_ADDR, DATA_DEFAULT, strlen(DATA_DEFAULT));
        ledBlink(PINK, 3);
        Serial << "Data set to default" << nl;
    }  

    encodeData(dataOut, bufferIn); 

    return true;
}

// ------------

void loop() 
{
    currentHug = huggiBuff.getAvail();

    if(currentHug == NULL)
    {
        Serial << "BUFF FULL!" << huggiBuff.getSize() << nl;
        ledBlink(RED, 4);
        delay(2000); 
    }
    else if(dataOut[0] == 0)
    {
        if(!setDataOut())
        {
            ledBlink(PINK, 3); // error
            Serial << "Sleeping." << nl;
            enterSleep();
            Serial << "Woken up." << nl;
        }
    }
    else if(sensor.isPressed())
    {
        ledSetColor(ORANGE);
        if(exchange(currentHug))
        {
            ledBlink(GREEN, 3);

            huggiBuff.commit();
            currentHug = huggiBuff.getAvail();
            sendHugs();
        }
        else
        {
            delay(500);
        }
    }
    
    delay(1000);
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
    long start = -1;

    long lastReceived = millis();
    bool dataReceived = false;
    bool otherHasReceivedData = false;

    int indexIn = -1;
    int indexOut = 0;


    Serial << "== Begin HUG..." << nl;


    while(sensor.isPressed())
    {
        long ms = millis() - lastReceived;
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
            {
                altSerial << (dataReceived ? DATA_OK : DATA_NOK);
            }

            altSerial << dataOut[indexOut++];
        }


        // ----------------------------- receive

        if(altSerial.available() )
        {

            if(start < 0) // first char received ever, start measuring
                start = millis();

            // get the next char
            char c = altSerial.read();
            Serial << c;
            lastReceived = millis();

            if(indexIn < 0) // first char: get the ack
            {
                otherHasReceivedData = (c == DATA_OK);
                indexIn = 0;
            }
            else if(dataReceived) // data already ok
            {
                // just note that we got and end
                // so we capture the ack on the next char
                if(c == nl) indexIn = -1;
            }
            else if(indexIn >= DATA_BUFF_SIZE - 1)
            {
                // buffer full, wait for the next data
                Serial << "dataIn full" << nl;
                if(c == nl) indexIn = -1;
                continue; 
            }
            else // data not received
            {
                bufferIn[indexIn++] = c;
                
                if(c == nl) // end of data -> process it
                {
                    bufferIn[--indexIn] = 0; // remove \n
                    indexIn = -1;

                    byte dataLen = decodeData(bufferIn, hug->id);
                    if(dataLen > 0) // decode ok
                    {                        
                        // split id and data, shift by one to add an "end of string"
                        memmove(hug->data, hug->data -1, dataLen - ID_SIZE + 1);
                        hug->id[ID_SIZE] = 0;

                        if(strcmp(dataOut, bufferIn) == 0) // detect loopbacks
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


    Serial << "== End of HUG... other|me:" << otherHasReceivedData << "|" << dataReceived << nl;

    if(dataReceived && otherHasReceivedData) // only if ok for both ? TODO
    {
        hug->duration = millis() - start;
        return true;
    }
    return false;
} // end exchange
