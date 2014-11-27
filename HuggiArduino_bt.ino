#define BT_CMD_PREFIX      '$'
#define BT_DATA_PREFIX     '@'
#define BT_ACK             '#'
#define BT_DATA_SEP        '!'
#define BT_TIMEOUT          2400

#include "HuggiArduino.h"


boolean setId()
{
    byte length = readData();
        Serial << "set id" << nl;

    if(length == ID_SIZE)
    {
        Serial << "set id" << nl;
        writeToEEprom(EEPROM_ID_ADDR, bufferIn, length);
        return true;
    }
    else
    {
        Serial << "Wrong id size: " << length << nl;
    }

    return false;
}


boolean setData()
{
    Serial << "set data" << nl;
    byte length = readData();

    if(length > 0)
    {
    Serial << "set data" << nl;
        writeToEEprom(EEPROM_DATA_ADDR, bufferIn, length);
        return true;
    }

    return false;
}


boolean sendHug()
{
    long lastSent;
    bool ok = true;

    Serial << huggiBuff.getSize() << nl;
    
    while(ok)
    {
        Hug_t * hug = huggiBuff.getNext(); // get one hug
        if(!hug) return false; // no more hug

        // send hug
        Serial << BT_DATA_PREFIX << 
            hug->id << BT_DATA_SEP <<
            hug->data << BT_DATA_SEP <<
            hug->duration << nl;

        ok = false;
        lastSent = millis();
        // wait ack
        while(millis() - lastSent < BT_TIMEOUT )
        {
            if(Serial.available() && (Serial.read() == BT_ACK) )
            {
                huggiBuff.remove();
                ok = true;
                break;
            }
        }   
    }

    return false; 
}

boolean echo()
{
    if(readData() > 0)
    {
        Serial << bufferIn << nl;
    }

    return true;

}


boolean bt()
{
    if( Serial.read() != BT_CMD_PREFIX)
    {
        Serial << "CMD wrong magic" << nl;
        readData(); // TODO: remove ? empty buffer until nl
        return false;
    }

    long lastReceived = millis();

    while(millis() - lastReceived < BT_TIMEOUT && 
        !Serial.available());

    switch (Serial.read()) {
        case 'I':
            Serial << 'I' << nl;
            return setId();

        case 'D':
            Serial << 'D' << nl;
            return setData();
        case 'C':
            Serial << 'C' << nl;
            sensor.calibrate();
            Serial << "calibration done\n";
            return true;

        case 'E':
            Serial << 'E' << nl;
            return echo();

        case 'S':
            while(Serial.available()) Serial.read();
            enterSleep();
            if(triggered)
            {
                Serial << "Triggered!" << nl;
                triggered = false;
            }
            return true;

        case 'H':
            Serial << 'H' << nl;
            bool ok = true;
            while(ok)
            {
                ok = sendHug();
            }
            return true;
        
    }

    readData(); // TODO: remove ? empty buffer until nl
    return false;

}




byte readData()
{
    byte i = 0;
    bufferIn[0] = 0; // reset
    long lastReceived = millis();
    char magic = 0;

    while(millis() - lastReceived < BT_TIMEOUT)
    {
        if(Serial.available())
        {
            char c = Serial.read();
            
            if(!magic) // first char
            {
                magic = c;
            }
            else if(c == nl) 
            {
                if(magic == BT_DATA_PREFIX)
                {
                    bufferIn[i] = 0;
                    return i;
                }
                Serial << "DATA wrong magic: " << bufferIn << nl;
                bufferIn[0] = 0; // reset
                return false;
            }
            else
            {
                bufferIn[i++] = c;
            }

            lastReceived = millis();
        }
   } 
   Serial << "DATA timeout " << millis() - lastReceived << nl;
   return 0;
}
