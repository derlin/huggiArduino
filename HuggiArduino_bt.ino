#define BT_CMD_PREFIX      '$'
#define BT_DATA_PREFIX     '@'
#define BT_ACK_PREFIX      '#'
#define BT_DATA_SEP        '!'
#define BT_TIMEOUT          2400


#define BT_CMD_ECHO         'E'
#define BT_CMD_SET_ID       'I'
#define BT_CMD_SET_DATA     'D'
#define BT_CMD_SEND_HUGS    'H'
#define BT_CMD_CALIBRATE    'C'
#define BT_CMD_SLEEP        'S'

#define BT_ACK_OK           '#'
#define BT_ACK_NOK          '?'

#include "HuggiArduino.h"

//#define SendOk()  do{ Serial < BT_ACK_PREFIX << BT_ACK_OK << nl; } while(1)
//#define SendNok() do{ Serial < BT_ACK_PREFIX << BT_ACK_NOK << nl; } while(1)

boolean setId()
{
    byte length = readData();
        Serial << "set id" << nl;

    if(length == ID_SIZE)
    {
        Serial << "set id" << nl;
        writeToEEprom(EEPROM_ID_ADDR, bufferIn, length);
        setDataOut(); // update
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
        setDataOut(); // update
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
        Serial << BT_DATA_PREFIX << BT_CMD_SEND_HUGS << BT_DATA_SEP <<
            hug->id << BT_DATA_SEP <<
            hug->data << BT_DATA_SEP <<
            hug->duration << nl;

        ok = false;
        lastSent = millis();
        // wait ack
        while(millis() - lastSent < BT_TIMEOUT )
        {
            if(Serial.available() && (Serial.read() == BT_ACK_PREFIX) )
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


void bt()
{
    if( Serial.read() != BT_CMD_PREFIX)
    {
        Serial << "CMD wrong magic" << nl;
    }
    else
    {
        long lastReceived = millis();
        bool sendAck = true;
        char cmd;
        bool ret;

        while(millis() - lastReceived < BT_TIMEOUT && 
            !Serial.available());

        cmd = Serial.read();

        switch (cmd) {
            case BT_CMD_SET_ID:
                Serial << 'I' << nl;
                ret = setId();
                break;

            case BT_CMD_SET_DATA:
                Serial << 'D' << nl;
                ret = setData();
                break;

            case BT_CMD_CALIBRATE:
                Serial << 'C' << nl;
                sensor.calibrate();
                Serial << "calibration done\n";
                ret = true;
                break;

            case BT_CMD_ECHO:
                Serial << 'E' << nl;
                ret = echo();
                break;

            case BT_CMD_SLEEP:
                while(Serial.available()) Serial.read();
                enterSleep();
                if(triggered) // pressure detected
                {
                    Serial << "Triggered!" << nl;
                    triggered = false;
                }
                sendAck = false;
                break;

            case BT_CMD_SEND_HUGS:
                Serial << 'H' << nl;
                bool ok = true;
                while(ok)
                {
                    ok = sendHug();
                }
                sendAck = false;
                break;      
        }

        if(sendAck)
        {
            Serial << BT_ACK_PREFIX << cmd << (ret ? BT_ACK_OK : BT_ACK_NOK);
            Serial << nl;
        } 

    }

    readData(); // TODO: remove ? empty buffer until nl
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
