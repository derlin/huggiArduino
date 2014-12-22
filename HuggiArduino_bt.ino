/**
 * @file    HuggiArduino_bt.ino
 * @author  Lucy Linder (lucy.derlin@gmail.com)
 * @date    September, 2014
 * @brief   Handle bluetooth communication.
 *
 * Handle the communication between the HuggiShirt and a 
 * bluetooth connected device.
 *
 * ## Communication protocol
 * 
 * The communication is made through the Serial interface of the
 * Arduino (RX = 8, TX = 9). All exchanges follow a basic protocol:
 *  - the connected device sends commands in the format: 
 *    > `$<command code>\n`
 *    
 *    for commands without argument, and
 *    > `$<command code>@<data>\n`
 *    for commands with parameters.
 *    
 *  - the Arduino executes the command and sends a feedback:
 *    > `##\n // ack`  \n
 *    > `#?\n // nak` 
 *
 *  In case of a return value, the latter will be prefixed with '@'.
 *
 * An exchange begins as soon as a #BT_CMD_PREFIX is received through the
 * Serial interface. It is ended as soon as the command has been executed,
 * or if the Arduino does not receive any data for more than #BT_TIMEOUT 
 * milliseconds.
 *
 * ## Special case: information about hugs
 *
 * The Arduino sends information about hugs everytime a new hug finishes,
 * whether or not a device is paired. 
 *
 * The information is in the form:
 * 
 * > `@<hugger id>!<data exchanged during the hug>!<duration>\n`
 * 
 * After each sending, the Arduino will listen for an ack (`#`, or #BT_ACK_PREFIX). Upon failure, the hug is stored in the buffer.
 * 
 * Information in the buffer may be queried anytime using the 
 * #BT_CMD_SEND_HUGS command.
 * 
 */
#define BT_TIMEOUT          2400    //!< Max delay between two received characters
#define BT_CMD_PREFIX      '$'      //!< Char prefixing any command  
#define BT_DATA_PREFIX     '@'      //!< Char prefixing a data block
#define BT_ACK_PREFIX      '#'      //!< Char prefixing an ack/nak
#define BT_DATA_SEP        '!'

#define BT_ACK_OK           '#'     //!< Notify command successful
#define BT_ACK_NOK          '?'     //!< Notify command failed

#define BT_CMD_ECHO         'E'     //!< Echo command code
#define BT_CMD_SET_ID       'I'     //!< ID configuration command code
#define BT_CMD_SET_DATA     'D'     //!< Data configuration command code
#define BT_CMD_SEND_HUGS    'H'     //!< Send hugs command code
#define BT_CMD_CALIBRATE    'C'     //!< Calibration command code
#define BT_CMD_SLEEP        'S'     //!< Sleep command code
#define BT_CMD_DUMP_ALL     'A'     //!< Dump current configuration command code
#define BT_CMD_RESET_EEPROM 'X'     //!< Reset EEPROM command code



#include "HuggiArduino.h"

//#define SendOk()  do{ Serial < BT_ACK_PREFIX << BT_ACK_OK << nl; } while(1)
//#define SendNok() do{ Serial < BT_ACK_PREFIX << BT_ACK_NOK << nl; } while(1)

/**
 * Send the current id and data configured in EEPROM.
 */
boolean dump_all()
{

    Serial << BT_DATA_PREFIX << BT_CMD_DUMP_ALL << BT_DATA_SEP; 

    readFromEEprom(EEPROM_ID_ADDR, bufferIn, DATA_BUFF_SIZE); // id
    Serial << bufferIn << BT_DATA_SEP;
    
    readFromEEprom(EEPROM_DATA_ADDR, bufferIn, DATA_BUFF_SIZE); // data
    Serial << bufferIn << nl;

    return true;
}

/**
 * Configure the ID.
 * Get the ID from Serial and write it to EEPROM if the 
 * size is correct (see #ID_SIZE).
 * @return true if a new ID has been set, false otherwise.
 */
boolean setId()
{
    byte length = readData();
        Serial << "set id" << nl;

    if(length == ID_SIZE)
    {
        Serial << "set id" << nl;
        writeToEEprom(EEPROM_ID_ADDR, bufferIn, length);
        setDataOut();
        return true;
    }
    else
    {
        Serial << "Wrong id size: " << length << nl;
    }


    return false;
}

/**
 * Configure the data which are sent during a hug.
 * Get the data from Serial and write it to EEPROM.
 * @return true if a new data has been set, false otherwise.
 */
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

/**
 * Try to send the hugs currently in the buffer through Serial.
 * Wait for an ack after each sending.
 * @return false
 */
boolean sendHugs()
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

/**
 * Echo the line received from Serial.
 * @return true if an echo was made, false otherwise.
 */
boolean echo()
{
    if(readData() > 0)
    {
        Serial << bufferIn << nl;
        return true;
    }

    return false;
}

/**
 * Handle bluetooth commands and communication.
 */
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
        bool ret = false;

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
                Serial << "Woken up." << nl;               
                return; // TODO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                // sendAck = false;
                // break;

            case BT_CMD_DUMP_ALL:
                Serial << 'A' << nl;
                ret = dump_all();
                break;  

            case BT_CMD_RESET_EEPROM:
                Serial << 'X' << nl;
                resetEEprom();
                dataOut[0] = 0;
                ret = true;
                break;    

            case BT_CMD_SEND_HUGS: // always at the end (or compil error !?!)
                Serial << 'H' << nl;
                while(true)
                {
                    if( !sendHugs() ) 
                        break;
                }
                sendAck = false;
                break; 

            default:
                Serial << cmd << " undefined." << nl;
                ret = false;
                sendAck = false;
        }

        if(sendAck)
        {
            Serial << BT_ACK_PREFIX << cmd << (ret ? BT_ACK_OK : BT_ACK_NOK);
            Serial << nl;
        } 

    }

    readData(); // TODO: remove ? empty buffer until nl
}



/**
 * Read one line of data.
 * If the data is correct (i.e. starts with :BT_DATA_PREFIX), it is 
 * stored in :bufferIn. Otherwise, the inputs are read and discarded.
 * 
 * @return the size of the data received, or 0 if the format was incorrect.
 */
byte readData()
{
    byte i = 0;
    bufferIn[0] = 0; // reset
    long lastReceived = millis();
    char magic = 0;

    while(millis() - lastReceived < BT_TIMEOUT && i < DATA_BUFF_SIZE)
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
                return 0;
            }
            else
            {
                bufferIn[i++] = c;
            }

            lastReceived = millis();
        }
   } 
   //Serial << "DATA timeout " << millis() - lastReceived << nl;
   return 0;
}
