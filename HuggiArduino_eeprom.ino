/**
 * @file    HuggiArduino_eeprom.ino
 * @author  Lucy Linder (lucy.derlin@gmail.com)
 * @date    December, 2014
 * @brief   Handle the storage of id and data in the EEPROM.
 *
 *
 * When a write occurs, we systematically write a magic, 'L', in the first
 * two cases of memory. 
 * When a read occurs, we check for the magic in order to avoid reading
 * garbage. This also ensure that we will eventually encounter an end-of-string (\0).
 * 
 * Reading from eeprom is relatively slow, 3,3 ms.
 * Write cycles are limited to 100'000, so be careful about how often you call #writeToEEprom.
 */

#define EEPROM_MAGIC    'L' // if configured, first 2 bytes are LL (avoid reading garbage)

// ========== EEPROM

/**
 * Write a string to eeprom.
 * 
 * address and address+1 will be set to magic and the 
 * string written afterwards.
 */
void writeToEEprom(byte address, char * data, int length)
{
    EEPROM.write(address, 0); // reset: not configured
    int i = 0;
    for(; i < length; i++) // write data 
    {
        EEPROM.write(address + 2 + i, data[i]);
        // Serial << "written: " << data[i] << " at " <<
             // address + 2 + i << nl;

    }
    EEPROM.write(address + 2 + i, 0); i++; // end of string
    
    // write magic
    EEPROM.write(address, EEPROM_MAGIC);
    EEPROM.write(address + 1, EEPROM_MAGIC);
}


/**
 * Read a string from the EEPROM, starting at address zero.
 * 
 * Note that if address and address+1 do not contain the
 * magic, nothing will be read and the return value will be 0.
 *
 * @param address the starting address  
 * @param data the buffer to store the string read
 * @param buff_size the size of the buffer
 * @return the length of the string read
 */
byte readFromEEprom(byte address, char * data, int buff_size)
{
    byte len = 0;

    if(EEPROM.read(address) != EEPROM_MAGIC || 
       EEPROM.read(address + 1) != EEPROM_MAGIC)
    {
        return len;
    }

    address += 2;
    char * stop = data + buff_size;

    for(; data < stop, address + len < EEPROM_MAX_ADDRESS; len++, data++)
    {
        *data = EEPROM.read(address + len);

        if(*data == 0) break; // end of string
    }

    return len;
}

/**
 * Reset the EEPROM
 * This function will erase the magic from the
 * id and data addresses, so that subsequent read will fail.
 */
void resetEEprom()
{
    EEPROM.write(EEPROM_ID_ADDR, 0);   // reset id
    EEPROM.write(EEPROM_DATA_ADDR, 0); // reset data
}